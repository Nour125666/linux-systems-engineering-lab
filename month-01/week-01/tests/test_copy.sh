
#!/usr/bin/env bash
set -euo pipefail

# Run from month-01/week-01
cd "$(dirname "$0")/.."

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT

# Compile
gcc -Wall -Wextra -Wpedantic -Wconversion \
    -g src/copy.c -o "$tmp/copy"

# Test 1: Small file
printf 'ABC' > "$tmp/input"
"$tmp/copy" "$tmp/input" > "$tmp/output"
cmp "$tmp/input" "$tmp/output"
echo "PASS: Small file"

# Test 2: Empty file
: > "$tmp/input"
"$tmp/copy" "$tmp/input" > "$tmp/output"
cmp "$tmp/input" "$tmp/output"
echo "PASS: Empty file"

# Test 3: Large binary file
head -c 10000 /dev/urandom > "$tmp/input"
"$tmp/copy" "$tmp/input" > "$tmp/output"
cmp "$tmp/input" "$tmp/output"
echo "PASS: Binary file"

# Test 4: Missing argument
if "$tmp/copy" > "$tmp/output" 2>"$tmp/error"; then
    echo "FAIL: Missing argument"
    exit 1
fi
echo "PASS: Missing argument rejected"

# Test 5: Nonexistent file
if "$tmp/copy" "$tmp/missing" > "$tmp/output" \
    2>"$tmp/error"; then
    echo "FAIL: Missing file"
    exit 1
fi
echo "PASS: Missing file rejected"

# Test 6: Write failure using /dev/full
printf 'ABC' > "$tmp/input"
if "$tmp/copy" "$tmp/input" > /dev/full \
    2>"$tmp/error"; then
    echo "FAIL: Write error not detected"
    exit 1
fi
grep -q "write:" "$tmp/error"
echo "PASS: Write error detected"

# Test 7: Directory read error
if "$tmp/copy" "$tmp" > "$tmp/output" \
    2>"$tmp/error"; then
    echo "FAIL: Read error not detected"
    exit 1
fi
grep -q "read:" "$tmp/error"
echo "PASS: Read error detected"

# Test 8: Input descriptor zero is valid
"$tmp/copy" "$tmp/input" <&- > "$tmp/output"
cmp "$tmp/input" "$tmp/output"
echo "PASS: File descriptor zero"

echo "ALL TESTS PASSED"

