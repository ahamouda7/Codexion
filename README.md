*This project has been created as part of the 42 curriculum by ahamouda*

> Before submitting: replace `your_login` here and in the C/header file headers
> with your actual 42 login. This package does not know your login.

## Description

Codexion is a C simulation based on subject version 1.5. Each imaginary coder
has a POSIX thread and follows this cycle:

1. Request the two neighbouring USB dongles.
2. Compile while holding both dongles.
3. Release them, starting their cooldown.
4. Debug, then refactor, then request the dongles again.

A separate monitor checks burnout deadlines. The program stops when one coder
burns out or when every coder has completed the required number of compiles.
It uses a custom binary min-heap for each dongle's pending requests. There are
no global variables and no libft dependency.

The production code is complete in this directory. `WALKTHROUGH.md` explains
the execution order; `VALIDATION.md` records what was tested and the limits
of those tests. `tests.py` is an optional development tool, not part of the C
program.

## Instructions

### Build

Use Linux or another environment with POSIX threads, a C compiler and Make.
On Windows, build inside WSL/Linux rather than the ordinary Windows terminal.

```sh
make
```

The executable is `codexion`. The Makefile uses `cc` with
`-Wall -Wextra -Werror -pthread` and links object files. Running `make` again
without changing any source or header does not rebuild or relink anything.

```sh
make clean    # Remove object files.
make fclean   # Remove object files and the executable.
make re       # Rebuild everything.
```

### Run

All eight arguments are mandatory, in this order:

```sh
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
    time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
| --- | --- |
| `number_of_coders` | Number of coder threads and dongles; at least 1. |
| `time_to_burnout` | Maximum time since the start of the coder's last compile, or simulation start. |
| `time_to_compile` | Time spent compiling while holding both dongles. |
| `time_to_debug` | Time spent debugging after releasing the dongles. |
| `time_to_refactor` | Time spent refactoring after debugging. |
| `number_of_compiles_required` | Completed compiles each coder must reach before collective success. |
| `dongle_cooldown` | Minimum time a released dongle remains unavailable. |
| `scheduler` | Exactly `fifo` or `edf`. |

All times are milliseconds. Numeric arguments must be decimal integers from
0 to `INT_MAX`, except the coder count must be positive. One leading `+` is
accepted. Empty values, negative values, spaces, fractions, overflow and
incorrect argument counts are rejected.

Zero has its ordinary meaning: zero phase/cooldown durations are allowed;
zero required compiles means the goal is already satisfied and produces no
state logs; zero burnout time with a positive goal means immediate burnout.
Arguments that describe impossible timing are simulated, not rejected.

```sh
./codexion 5 2000 100 50 50 3 20 edf
./codexion 5 2000 100 50 50 3 20 fifo
./codexion 1 200 100 50 50 3 20 edf
```

The first two examples have generous timing margins. The one-coder example
must burn out: one dongle cannot satisfy the need for two distinct dongles.
Coder IDs in logs run from 1 to N. Timestamps are elapsed milliseconds since
the common simulation start. Thread scheduling can change the order of logs
between separate runs.

Exit status is 0 for a normal simulation end, including burnout. Invalid
arguments or initialization/runtime failures return 1 and print an error
to standard error. State logs use standard output.

### Check

```sh
python3 tests.py
norminette
```

Run timing tests on an otherwise idle machine. Run memory/race tools
separately: their overhead changes the simulated timing.

## Scheduling

Each dongle has its own mutex, busy flag, cooldown timestamp and min-heap.
The heap can hold two requests: only the two adjacent coders can request
that dongle, and each coder has at most one outstanding request per dongle.
For two coders, each coder requests both distinct dongles.

One request is copied into both neighbouring heaps while the shared state
mutex is locked. That makes insertion one coordinated operation.

| Policy | Order used by the heap |
| --- | --- |
| FIFO | Smaller arrival sequence first; coder ID breaks any remaining tie. |
| EDF | Earlier `last_compile + time_to_burnout` first; then arrival sequence; then coder ID. |

The sequence increases at request insertion, not at thread creation. EDF
deadlines stay unchanged while a request waits. A new deadline is calculated
for the coder's next request after it has started another compile.

A coder can claim its pair only when its request is first in **both** heaps,
both dongles are free, and both cooldowns have expired. Both requests are
popped and both busy flags are set in the same protected operation. A free
dongle can therefore stay idle while its highest-priority requester waits
for its other dongle. A lower-priority requester cannot skip that request.
Coders with disjoint pairs can compile concurrently.

## Blocking cases handled

### Deadlock and Coffman's conditions

Deadlock requires four conditions together: mutual exclusion, hold and wait,
no preemption, and circular wait. Mutual exclusion and no preemption are
appropriate for dongles, but a coder never reserves one dongle while waiting
for its second. Both are granted together. This breaks hold and wait for
dongle ownership.

The implementation also has a fixed mutex order: state mutex first, then the
lower-address dongle mutex, then the higher-address dongle mutex, then the
output mutex if needed. The dongles belong to one allocated array, so their
pointer ordering is defined in C. No code takes these locks in reverse order.
The one-coder path locks its single dongle only once.

### Starvation and EDF

The same total request order is used in every heap. Dependencies between
waiting requests therefore cannot form a priority cycle: following a
dependency always leads to a strictly earlier request. The earliest pending
request can proceed after any current holders finish and cooldowns expire.

An EDF request keeps its deadline while waiting. A coder that gets a turn
updates its last-compile time; its next request cannot repeatedly overtake
an older waiting request. Equal deadlines use the arrival sequence so repeated
requests with zero-duration phases cannot win ties forever. This prevents
indefinite postponement by queue arbitration, assuming runnable threads
eventually receive CPU time and output does not block indefinitely.

Finite waiting is different from meeting every deadline. Compile, debug,
refactor, contention, cooldown and OS delays must fit the burnout budget.
For example, when compiles are still required, a compile duration greater
than or equal to the burnout duration cannot complete before burnout.
This implementation does not claim an optimal schedule or a proof that every
parameter set feasible under some other schedule will meet its deadlines.
The subject's broad EDF feasibility requirement needs to be assessed with
the evaluator's timing cases; the completed features and passing tests alone
do not establish that universal claim.

### Cooldown and waiting

Release sets `available_at = elapsed_release_time + dongle_cooldown` for each
dongle. Acquisition checks the current time against both timestamps. A waiter
uses a timed condition wait when a cooldown is still running, so no extra
thread needs to announce that time has passed. It rechecks its conditions
after every wakeup.

### Burnout and stopping

The monitor waits until the earliest known absolute burnout deadline and
rescans when a compile starts. All coders share the same start time and
initial `last_compile = 0`.

`next_deadline` is a cached lower bound on the earliest deadline. Starting a
compile can only move a deadline later. If a worker finds this cached time
has arrived, it wakes the monitor and waits for it to check the state before
performing another action. This prevents a late worker from hiding a burnout
by refreshing its deadline or declaring collective success first.

The monitor sets the stop flag and writes exactly one burnout line while
holding the state mutex. It wakes every waiter. Workers stop and return;
the main thread joins them before freeing anything. No normal state logs
are produced after that burnout line.

The code uses `gettimeofday()` and the default realtime clock for timed
condition waits. Do not change the system clock during a run. Burnout logs
are written directly with `write()` to avoid stdio buffering. The design
targets the subject's 10 ms reporting limit, but a general-purpose OS cannot
give an unconditional wall-clock guarantee under arbitrary CPU overload or
a blocked output destination. See the measured results in `VALIDATION.md`.

### Completion and failures

A completed compile increments a coder's counter until it reaches the
required value. The counter is capped to avoid integer overflow. Reaching
the goal does not retire an individual coder: it continues its cycle until
everyone has reached the goal or someone burns out. Burnout checks continue
to cover every coder.

Allocation and initialization failures clean up only the resources already
initialized. A partial thread-creation failure stops and wakes all threads
that were created, joins them, then destroys mutexes, the condition variable,
heaps and arrays. Only the main thread joins workers, once each, using IDs
from successful `pthread_create()` calls; no thread is detached or cancelled.
Thus the join calls satisfy their POSIX preconditions.

## Thread synchronization mechanisms

| Mechanism | Protected state / purpose |
| --- | --- |
| `state_mutex` | Start/stop/error flags, compile times/counts, request sequence, collective completion, monitor deadline and coordinated pair decisions. |
| Each dongle's `mutex` | Its heap, `in_use` flag and cooldown time. |
| `print_mutex` | A complete log line, including repeated `write()` calls after a short write. |
| `changed` condition variable | Startup, resource releases, compile starts, deadline refreshes and stopping. Always paired with `state_mutex`. |
| `pthread_join` | Ensures a created thread has finished before its data is destroyed. |

For example, a coder writes `last_compile` while holding `state_mutex`, and
the monitor reads it while holding that same mutex. Neither accesses a
partially coordinated update. A dongle's heap is changed only while its
mutex is locked. The code retains logical ownership through `in_use` while
compiling; it does **not** keep a pthread mutex locked for the whole compile.

`pthread_cond_wait()` and `pthread_cond_timedwait()` release `state_mutex`
while sleeping, then reacquire it before returning. This lets other coders
and the monitor work. Wakeups are always followed by a condition recheck;
a wakeup itself does not give a coder permission to take a dongle.
Broadcasts tell waiting threads to recheck shared state. Coders do not send
messages to one another or inspect another coder's personal deadline.

## Resources

- The supplied **Codexion subject, version 1.5**: project requirements.
- [Linux manual: pthread_create](https://man7.org/linux/man-pages/man3/pthread_create.3.html)
- [Linux manual: pthread_join](https://man7.org/linux/man-pages/man3/pthread_join.3.html)
- [POSIX manual: mutex locking](https://man7.org/linux/man-pages/man3/pthread_mutex_lock.3p.html)
- [POSIX manual: condition waits](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html)
- [Official 42 Norminette](https://github.com/42School/norminette)
- Cormen, Leiserson, Rivest and Stein, *Introduction to Algorithms*: binary
  heaps and priority queues.

### AI use

ChatGPT/Codex was used to generate and revise the C implementation, Makefile,
documentation and optional tests; review synchronization and failure paths;
run compilation, Norm and runtime checks; and prepare explanations of the
code. This includes the parser, timing, heap, scheduler, thread lifecycle,
monitor, logging and cleanup. The student must study and verify the code and
be able to explain and modify it during evaluation. This disclosure describes
the assistance used; it does not claim the student has already completed that
study or that AI testing replaces evaluation.
