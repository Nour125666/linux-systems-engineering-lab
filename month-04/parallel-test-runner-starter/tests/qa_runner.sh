#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")/.."

gcc -std=c11 -Wall -Wextra -Wpedantic -Werror \
    -g -O0 -pthread ptrunner.c -o ptrunner

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

printf '%s\n' 'true' 'test 10 -gt 5' 'true' > "$tmp/pass.txt"
printf '%s\n' 'true' 'false' 'true' 'exit 7' > "$tmp/mixed.txt"
: > "$tmp/empty.txt"

for ((i=0; i<40; i++)); do
    echo true
done > "$tmp/many.txt"

check() {
    local name="$1"
    local expected="$2"
    local summary="$3"
    shift 3

    local rc=0

    timeout 15s "$@" > "$tmp/output.txt" 2>&1 || rc=$?

    if (( rc != expected )); then
        echo "FAIL: $name (exit $rc, expected $expected)"
        cat "$tmp/output.txt"
        exit 1
    fi

    if ! grep -Fq "$summary" "$tmp/output.txt"; then
        echo "FAIL: $name (incorrect summary)"
        cat "$tmp/output.txt"
        exit 1
    fi

    echo "PASS: $name"
}

check "Sequential passing tests" 0 \
    "Summary: 3 passed, 0 failed, 3 total" \
    ./ptrunner "$tmp/pass.txt"

check "Sequential mixed tests" 1 \
    "Summary: 2 passed, 2 failed, 4 total" \
    ./ptrunner "$tmp/mixed.txt"

check "Parallel passing tests" 0 \
    "Summary: 3 passed, 0 failed, 3 total" \
    ./ptrunner --parallel "$tmp/pass.txt"

check "Parallel mixed tests" 1 \
    "Summary: 2 passed, 2 failed, 4 total" \
    ./ptrunner --parallel "$tmp/mixed.txt"

check "Parallel empty manifest" 0 \
    "Summary: 0 passed, 0 failed, 0 total" \
    ./ptrunner --parallel "$tmp/empty.txt"

check "Queue wraparound: 40 jobs" 0 \
    "Summary: 40 passed, 0 failed, 40 total" \
    ./ptrunner --parallel "$tmp/many.txt"

count=$(grep -Ec '^\[Worker [0-9]+\] PASS: true$' \
    "$tmp/output.txt" || true)

if (( count != 40 )); then
    echo "FAIL: Expected 40 worker result lines, found $count"
    exit 1
fi

echo "PASS: 40 worker results recorded"

for ((i=1; i<=100; i++)); do
    timeout 5s ./ptrunner --queue-selftest \
        > "$tmp/output.txt" 2>&1 || {
        echo "FAIL: Queue self-test iteration $i"
        cat "$tmp/output.txt"
        exit 1
    }
done

echo "PASS: 100 queue self-test iterations"
echo
echo "QA STAGE 1 PASSED"
