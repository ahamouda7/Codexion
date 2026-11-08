# Validation of this version

Checked on 8 November 2026 in a Linux environment with GCC 13.3.0.
These results describe the tested executions, not all possible thread
schedules or a guarantee of an evaluation grade.

| Check | Result |
| --- | --- |
| Build with `cc -Wall -Wextra -Werror -pthread` | Passed. |
| Repeated unchanged `make` | Object and executable timestamps unchanged. |
| Norminette 3.3.60 | All 17 C files and 2 headers passed. |
| Included `python3 tests.py` | 108 cases passed. |
| Larger/sustained contention runs | 6 additional runs passed, including 200 coders with each policy. |
| Heap tests | FIFO/EDF ordering, 64-element insertion/removal, full/empty queues, ties and reuse passed. |
| Scheduler integration checks | Different FIFO/EDF winners, two neighbouring heaps, busy flags and cooldown boundaries passed. |
| Burnout display checks | 20 one-coder runs with a 30 ms deadline; every burnout timestamp was within the 10 ms allowance, and the streamed line arrived within that allowance relative to the first received log. |
| AddressSanitizer + UndefinedBehaviorSanitizer | 4 representative runs passed with leak detection disabled. |
| ThreadSanitizer | 4 representative runs passed without reported data races or lock-order inversions. |
| Allocation/resource tracking with failure injection | 23 paths passed: ordinary completion, each of 8 program allocation failures, 7 mutex initialization failures, the condition initialization failure, and each of 6 thread creation failures. All created threads joined and all initialized resources released. |
| Valgrind Memcheck 3.22.0 | 28 runs passed: 5 normal/edge cases plus those 23 tracked paths. Every run reported zero bytes in use at exit and zero errors. |

The included Python checks inspect log format, phase order/durations,
neighbouring dongle exclusion, minimum compile-plus-cooldown spacing, the
compile goal, stopping and invalid inputs. They cannot directly observe
internal queue arrivals; those were checked separately through the C heap
and scheduler interfaces.

The sanitizer cases covered FIFO, EDF, a single coder and zero-duration
phases. Valgrind additionally covered burnout during compiling and a zero
compile goal. Timing was tested separately from instrumentation.

The development environment could not run LeakSanitizer's exit-time scan
because of its process-inspection restrictions. Address/undefined-behavior
checks therefore used `ASAN_OPTIONS=detect_leaks=0`. Valgrind supplied the
independent leak checks using a private matching glibc/debug-symbol setup;
no runtime dependencies from that setup are included in the project.

## Reproduce the basic checks

```sh
make
python3 tests.py
norminette
make
```

The final `make` should report that no build work is needed.

For memory checking on a Linux machine with Valgrind installed:

```sh
valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 \
    ./codexion 5 10000 10 5 5 3 5 edf
```

For a separate race-detector build:

```sh
cc -Wall -Wextra -Werror -pthread -g -O1 -fsanitize=thread \
    -fno-omit-frame-pointer *.c -o codexion_tsan
./codexion_tsan 5 10000 10 5 5 3 5 edf
rm -f codexion_tsan
```

Use a generous burnout time under instrumentation. A slow instrumented run
is not a suitable test of the 10 ms real-time reporting requirement.

## Limits and submission details

- The FIFO/EDF arbitration rules and finite-waiting argument are documented
  in the README. Neither these tests nor that argument proves that every
  parameter set feasible under some schedule will meet its EDF deadlines in
  this implementation. This remains a limit on claiming complete subject
  compliance; no universal schedulability proof is supplied.
- Test the final program on the evaluation machine. OS load, thread limits,
  realtime clock changes and a blocked output stream can affect timing.
- Replace `your_login` in the README and regenerate the 42 source/header
  headers with your identity; rerun Norminette afterward.
- Submit the project directory's contents to the assigned Git repository.
  The archive itself is a delivery format; it does not submit the project.
- `tests.py` and the two learning/validation documents are optional aids.
  The executable is built only from the C sources listed in the Makefile.
