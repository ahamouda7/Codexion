#!/usr/bin/env python3
"""Optional black-box checks. Run `make` and then `python3 tests.py`."""

import re
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parent
PROGRAM = ROOT / "codexion"
LINE = re.compile(
    r"^(\d+) (\d+) (has taken a dongle|is compiling|is debugging|"
    r"is refactoring|burned out)$"
)
CHECKS = 0


def execute(args):
    return subprocess.run(
        [str(PROGRAM), *map(str, args)],
        capture_output=True,
        text=True,
        timeout=15,
    )


def invalid_arguments():
    global CHECKS
    valid = [3, 1000, 10, 5, 5, 3, 5, "edf"]
    cases = [[], valid[:-1], valid + ["extra"]]
    for position in range(7):
        for value in ["-1", "1.5", "x", "", "+", " 1", "2147483648"]:
            args = valid.copy()
            args[position] = value
            cases.append(args)
    cases += [[0, *valid[1:]], [*valid[:-1], "EDF"]]
    for args in cases:
        result = execute(args)
        assert result.returncode == 1, (args, result)
        assert not result.stdout, (args, result.stdout)
        assert "Error: invalid arguments" in result.stderr, result.stderr
        CHECKS += 1


def simulation(args, expect_burnout=False):
    global CHECKS
    count, burnout, compile_ms, debug, refactor, required, cooldown, _ = args
    result = execute(args)
    assert result.returncode == 0 and not result.stderr, (args, result)
    last = [0] * count
    starts = [0] * count
    dongle_start = [None] * count
    phase = ["request"] * count
    phase_time = [0] * count
    takes = [0] * count
    burned = False
    previous_time = 0
    for line in result.stdout.splitlines():
        match = LINE.fullmatch(line)
        assert match, (args, line)
        now, coder, event = match.groups()
        now, index = int(now), int(coder) - 1
        assert 0 <= index < count, line
        assert now >= previous_time and not burned, (args, line)
        previous_time = now
        if event == "burned out":
            assert now >= min(last) + burnout, (args, line)
            assert last[index] == min(last), (args, line, last)
            burned = True
            continue
        assert now < min(last) + burnout, (args, line, last)
        if event == "has taken a dongle":
            if phase[index] == "refactor":
                assert now >= phase_time[index] + refactor, (args, line)
            assert phase[index] in ("request", "refactor"), (args, line)
            takes[index] += 1
        elif event == "is compiling":
            assert count > 1 and takes[index] == 2, (args, line)
            for dongle in (index, (index - 1) % count):
                start = dongle_start[dongle]
                if start is not None:
                    assert now >= start + compile_ms + cooldown, (args, line)
                dongle_start[dongle] = now
            takes[index] = 0
            last[index] = now
            starts[index] += 1
            phase[index] = "compile"
            phase_time[index] = now
        elif event == "is debugging":
            assert phase[index] == "compile", (args, line)
            assert now >= phase_time[index] + compile_ms, (args, line)
            phase[index] = "debug"
            phase_time[index] = now
        else:
            assert phase[index] == "debug", (args, line)
            assert now >= phase_time[index] + debug, (args, line)
            phase[index] = "refactor"
            phase_time[index] = now
    assert burned == expect_burnout, (args, result.stdout)
    if not burned:
        assert all(n >= required for n in starts), (args, starts)
    CHECKS += 1


def main():
    invalid_arguments()
    for policy in ("fifo", "edf"):
        for count in (2, 3, 4, 5, 8, 16):
            for _ in range(3):
                simulation([count, 3000, 4, 3, 2, 6, 3, policy])
        simulation([5, 1000, 0, 0, 0, 40, 0, policy])
        simulation([3, 1000, 0, 0, 0, 5, 10, policy])
        simulation([3, 1000, 5, 0, 0, 0, 0, policy])
        simulation([1, 30, 10, 5, 5, 2, 5, policy], True)
        simulation([2, 30, 60, 5, 5, 2, 5, policy], True)
        simulation([2, 30, 2, 60, 5, 2, 0, policy], True)
        simulation([2, 30, 2, 2, 60, 2, 0, policy], True)
        simulation([2, 30, 2, 2, 2, 2, 60, policy], True)
        simulation([3, 0, 5, 5, 5, 2, 5, policy], True)
    print(f"PASS: {CHECKS} cases (parsing, phases, dongles, cooldown, stopping)")
    print("These log checks do not prove every interleaving or EDF feasibility.")


if __name__ == "__main__":
    main()
