# Read the real code in execution order

Use this guide beside the source. We will study one section at a time.
You do not need to learn a separate demonstration program first.

One useful distinction: the `t_heap` priority queue is a data structure.
The memory sometimes called "the heap" is where `malloc()` allocates memory.
These are two different meanings of the same word.

## 1. Start in main.c

```c
int main(int argc, char **argv)
```

`argc` is how many command-line strings we received. `argv` is the array of
those strings. The executable name counts as one string, so eight project
arguments means `argc == 9`.

```c
t_config config;
```

This creates one configuration variable. It contains the input values,
including the selected scheduler. At this point its fields are not filled.

```c
if (!parse_arguments(argc, argv, &config))
```

We give the parser the address of `config` so it can fill that variable.
Our parser returns 1 for success and 0 for failure. `!` means "not", so the
body of this `if` runs when parsing fails. It writes an error and returns
1 from `main`, ending the process with a failure status.

```c
if (!run_simulation(config))
```

This line runs only after parsing succeeds. It passes a copy of the filled
configuration into the simulation. Another return value of 0 means failure.
If both calls succeed, `main` returns 0.

Do not confuse conventions: **our helper functions** generally return 1 on
success; **pthread functions** return 0 on success. `main` also returns 0
for success. Always check the particular function's contract.

## 2. Read parse.c

`parse_arguments()` first checks the argument count. It calls `parse_number()`
for each numeric argument and `parse_scheduler()` for the final string.

`parse_number()` optionally skips one `+`, rejects an empty string, then
checks one character at a time. Subtracting `'0'` converts a digit character
to its numeric value. For example, `'7' - '0'` gives 7.

Before calculating `value * 10 + digit`, it checks:

```c
if (value > (INT_MAX - digit) / 10)
    return (0);
```

The multiplication would be too large if this condition is true. Checking
before multiplying avoids signed integer overflow. It writes through
`*result` only after the whole string is valid.

`parse_scheduler()` uses `strcmp()` to compare text. It explicitly translates
the string `"fifo"` to the enum constant `FIFO`, and `"edf"` to `EDF`. C does
not automatically connect strings with enum names.

Negative input is already rejected by `parse_number()`. The additional
`number_of_coders == 0` check rejects the remaining invalid coder count.

## 3. Enter run_simulation() in simulation.c

The first action is:

```c
sim = create_simulation(config);
```

Follow that call into `setup.c` before reading the next line. A function call
temporarily takes execution into that function; execution returns to the
caller when it finishes.

## 4. Allocate and initialize: setup.c, dongles.c, heap.c

`create_simulation()` allocates the shared `t_simulation` object and the array
of `t_coder` objects. `malloc()` returns an address or `NULL` on failure.
`memset()` zeroes the simulation's ordinary storage before the mutexes and
condition variable are explicitly initialized. Zeroing does not replace
`pthread_mutex_init()` or `pthread_cond_init()`.

The allocation size check uses division before multiplication:

```c
if ((size_t)config.number_of_coders > SIZE_MAX / sizeof(t_coder))
    return (NULL);
```

`size_t` is the unsigned type used for allocation sizes. `sizeof(t_coder)`
is the number of bytes for one coder. The check ensures the multiplication
for the whole array will fit.

`init_locks()` creates the state mutex, condition variable and output mutex.
If a later initialization fails, it destroys the earlier successful ones.

`init_dongles()` allocates the dongle array. For each dongle, `init_dongle()`
initializes its mutex and calls `heap_init()` with capacity 2. `in_use = 0`
means free; `available_at = 0` means no initial cooldown.

The heap stores `t_request` values in an allocated array. Its `size` starts
at 0. `capacity` is the maximum number of stored requests. `policy` tells
the heap which comparison to use. Each dongle has at most two neighbouring
requesters, so capacity 2 is enough. The heap functions themselves also
work with larger capacities.

Once initialization succeeds, execution returns to `run_simulation()`.

## 5. Create threads: simulation.c and init_coder()

`launch_threads()` calls `init_coder()` before creating each coder thread.

`init_coder()` sets the ID, simulation pointer, zero compile count and zero
last-compile time. For array index `i`, the first dongle is `dongles[i]` and
the second is `dongles[i - 1]`, wrapping index 0 to index N - 1. Logs use
`id = i + 1`. A single coder gets `second_dongle = NULL`.

```c
pthread_create(&sim->coders[i].thread, NULL,
    coder_routine, &sim->coders[i])
```

| Part | Meaning |
| --- | --- |
| `&sim->coders[i].thread` | Where pthread_create stores the new thread's identifier. |
| `NULL` | Use default thread attributes. |
| `coder_routine` | Function the new thread will run. It is passed, not called here. |
| `&sim->coders[i]` | Address passed as that function's `arg`. |

The array stays allocated until all threads have been joined. We do not pass
the address of the loop variable `i` as coder data.

After all coder threads are created, the main thread creates the separate
monitor thread with `monitor_routine` and the simulation pointer.

`start_simulation()` records the common start time, sets the first deadline,
sets `started`, and broadcasts the condition variable. If creation failed,
it sets `failed` and `stopped` instead, then wakes any created workers so
they can exit. A zero compile goal sets `stopped` immediately.

The main thread then waits in `pthread_join()`. The workers execute the next
sections concurrently; there is no single fixed order between different
threads.

## 6. Worker startup: coder.c

`coder_routine()` receives a `void *`, which can hold the address of any
object. Here that address points to a `t_coder`:

```c
coder = (t_coder *)arg;
sim = coder->simulation;
```

`coder->simulation` means "follow the coder pointer, then read its simulation
field". The cast makes the intended pointed-to type explicit; C also allows
this conversion from `void *` without a cast.

`wait_for_start()` locks the state mutex and waits while the simulation is
neither started nor stopped. A condition wait releases this mutex while
sleeping and reacquires it before returning. The `while` checks the flags
again because a wakeup does not prove that the start condition is true.

The worker then enters its real cycle:

```c
while (take_dongles(coder))
```

This repeats only when the coder successfully gets both dongles.

## 7. Join the queues: scheduler.c and queue.c

`take_dongles()` locks `state_mutex`, checks the stop flag, then chooses the
single-coder path or the normal two-dongle path.

For one coder, `wait_alone()` marks the only dongle busy and logs one take,
then waits for the monitor to stop the simulation. It cannot compile.

For multiple coders, `join_queue()` locks both dongle mutexes using
`lock_pair()`. Every caller locks the lower array address first. This keeps
the order consistent, including the pair that wraps around the ring.

`enqueue_request()` fills one `t_request`:

```c
request.coder_id = coder->id;
request.arrival_order = sim->next_order++;
request.deadline = coder->last_compile + sim->config.time_to_burnout;
```

`next_order++` uses the old sequence value for this request, then increases
the sequence for the next request. The state mutex prevents two threads
from assigning the same sequence. The code checks the sequence and heap
capacities before inserting anything.

`heap_push()` copies this request into each neighbouring dongle's heap. These
are two copies of the same request data. They do not create another thread
or allocate a new coder. Then `unlock_pair()` releases the dongle mutexes.

## 8. Understand the heap when it is first used

Read `request_before()` in `heap_utils.c` first. It answers: "should request
A be served before request B?" EDF compares deadlines, then arrival order,
then coder ID. FIFO compares arrival order, then coder ID.

Read `heap_push.c` next. A new request starts at the end of the array. Its
parent index is `(index - 1) / 2`. If it belongs before its parent, the code
swaps them and repeats upward. Thus the earliest request is always at index 0.

`heap_peek()` reads that first request without removing it.

`heap_pop()` removes the first request. It moves the last array item into
index 0, then moves that item downward as needed. The children of index `i`
are `2 * i + 1` and `2 * i + 2`. `first_child()` chooses the child that has
higher priority. This restores the same heap rule after removal.

`swap_requests()` uses a temporary structure to exchange two values.
`heap_destroy()` frees the array after the simulation ends.

## 9. Wait, then claim the pair

Back in `wait_for_pair()`, the worker asks `simulation_time()` for the current
time after any due monitor check. If stopped, it exits.

`pair_ready()` checks both heap roots, both busy flags and both cooldown
timestamps. All of these conditions must be true together. If ready,
`claim_pair()` pops both requests, marks both dongles busy, records the
compile start, and logs two takes followed by `is compiling`.

The state mutex makes the full decision indivisible relative to other
workers and the monitor. The dongle mutexes protect each dongle's own state.
Both kinds of mutex are released before the simulated compile duration.
The busy flags preserve ownership during that duration.

If the pair is not ready, `wait_for_change()` sleeps on the condition
variable. If a cooldown is still running, it uses the next cooldown expiry
as a timeout. Otherwise it waits for a state change, such as a release.
After waking, the loop checks everything again.

## 10. Compile, release, debug and refactor: phase.c

`perform_phase()` waits until a phase's end time or until the simulation
stops. It keeps waiting after early/spurious wakeups; a notification does
not shorten the phase.

For compiling, the message argument is `NULL`: the scheduler has already
printed `is compiling`. Its end time is based on the recorded compile start.
For debugging/refactoring, the function prints the phase message and waits
from that phase's start time.

`finish_compile()` first lets the monitor handle any expired deadline,
then calls `release_dongles()`. Release clears both busy flags, sets both
cooldown times, and broadcasts that resources changed.

If still running, the completed-compile count increases until it reaches
the required value. On reaching the value for the first time, the shared
`finished` count increases. When `finished` equals the coder count, the
simulation stops and wakes all waiters. The capped counter avoids overflow.

If the simulation is still running, `coder_routine()` performs debugging,
then refactoring, then returns to `take_dongles()`. If stopped during a
compile, the worker still releases its pair before returning.

## 11. The monitor runs alongside the workers: monitor.c

`monitor_routine()` also waits for the common start. It holds `state_mutex`
whenever it reads the shared compile times or changes the stop flag.

`earliest_coder()` finds the smallest `last_compile`. All coders have the
same burnout duration, so this coder also has the earliest deadline.
Equal times choose the smaller ID because the array is scanned in order.

`check_coders()` adds the start time and burnout duration to get an absolute
deadline. If the current time has reached it, the monitor sets `stopped`,
logs `burned out`, and wakes everyone. Otherwise it refreshes the cached
`next_deadline` if needed. The monitor then waits until that deadline or
another change.

`simulation_time()` in `wait.c` makes workers wait for this check when the
cached deadline has arrived. This matters even if the worker itself is
healthy: another coder might already be due to burn out. Workers only use
the shared deadline bound; they do not read another coder's individual data.

`fail_simulation()` also sets the stop flag and broadcasts, but records
`failed = 1` so `main()` can report an internal failure.

## 12. Time and output helpers

`current_time_ms()` in `time.c` gets a `struct timeval`:

```c
return ((long long)now.tv_sec * 1000 + now.tv_usec / 1000);
```

Seconds become milliseconds by multiplying by 1000. Microseconds become
milliseconds by dividing by 1000. The cast happens before multiplication so
the calculation uses the wider integer type. Subtracting `start_ms` gives
elapsed time. `last_compile` and `available_at` are elapsed times;
`next_deadline` and condition-wait targets are absolute times.

`wait_until()` converts an absolute millisecond target to `struct timespec`:
whole seconds and a remaining nanosecond part. `ETIMEDOUT` is an expected
result, not a simulation failure. Other errors stop the simulation.

`log_state()` in `log.c` builds a line in a local 80-byte array. This is large
enough for the numeric fields and the fixed subject messages. It locks the
print mutex, calls `write_line()`, then unlocks. `write_line()` handles short
writes and retries an interrupted write; it reports other failures.

`append_number()` puts integer digits into that array. It calls itself with
`number / 10` to put the earlier digits first, then appends the last digit
using `number % 10`. This is recursion. For 123, it processes 12, then 1,
then appends 1, 2 and 3 while those calls return. The pointer `length` lets
all calls update the same position in the local array.

This small formatter allows direct `write()` output, including when stdout
is redirected. The burnout line is not held in a stdio buffer.

## 13. Return to the main thread and clean up

The main thread's joins return as the workers finish. It joins every coder
that was successfully created, then the monitor if it was created.

`destroy_simulation()` destroys every dongle heap/mutex, frees the dongle
array, destroys the shared condition variable/mutexes, then frees the coder
array and simulation object. Cleanup happens after joins so no worker can
access freed data.

`run_simulation()` returns its success/failure result to `main()`. Finally,
`main()` returns the process exit status.

## What to understand before evaluation

| Concept | Actual place to explain it |
| --- | --- |
| Structs, pointers, `&`, `*`, `->`, arrays | `codexion.h`, `main.c`, `init_coder()` |
| Enum values versus text | `parse_scheduler()` |
| Allocation, ownership, overflow checks | `create_simulation()`, `heap_init()`, cleanup |
| Function pointers, thread arguments, joins | `launch_threads()`, `coder_routine()` |
| Shared data and data races | `last_compile` updates and monitor reads |
| Mutex locking and consistent lock order | `lock_pair()`, `take_dongles()` |
| Condition waits, timeouts, wakeup rechecks | `wait.c`, `perform_phase()` |
| FIFO, EDF, deterministic tie breaking | `request_before()` |
| Binary heap insertion/removal | `heap_push()`, `heap_pop()` |
| Deadlock, finite waiting, deadline feasibility | README blocking cases |
| Wall-clock time and elapsed time | `current_time_ms()`, monitor deadlines |
| Serialized output, recursion, partial writes | `log.c` |
| Partial failure cleanup | `init_locks()`, `init_dongles()`, `launch_threads()` |
| Incremental compilation and linking | `Makefile` |

Practise tracing one coder through a full cycle, explaining which mutex is
held at each shared-data access, and explaining the one-coder case. The
optional Python tests are a checking tool; begin your study with the C code.
