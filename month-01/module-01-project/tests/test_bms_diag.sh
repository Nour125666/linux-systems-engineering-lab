
#!/usr/bin/env bash

set -u

# The Makefile compiles bms_diag before running this script.
# Run the suite using: make test

# Prepare binary test inputs.
printf '\x4E\x52\x01\x00\x34\x12\xAB\xCD' > valid.bin
printf '\x00\x52\x01\x00\x34\x12\xAB\xCD' > bad_magic.bin
printf '\x4E\x52\x01' > short.bin

failures=0

# Test 1: Valid frame
if ./bms_diag valid.bin > test_output.txt 2>&1 &&
   grep -q 'Voltage: 4660 mV' test_output.txt; then
    echo "PASS: valid frame"
else
    echo "FAIL: valid frame"
    failures=$((failures + 1))
fi

# Test 2: Invalid identifier
./bms_diag bad_magic.bin > test_output.txt 2>&1
code=$?

if [ "$code" -eq 1 ] &&
   grep -q 'Invalid identifier' test_output.txt; then
    echo "PASS: invalid identifier"
else
    echo "FAIL: invalid identifier (exit $code)"
    failures=$((failures + 1))
fi

# Test 3: Truncated frame
./bms_diag short.bin > test_output.txt 2>&1
code=$?

if [ "$code" -eq 1 ] &&
   grep -q 'Invalid frame length' test_output.txt; then
    echo "PASS: truncated frame"
else
    echo "FAIL: truncated frame (exit $code)"
    failures=$((failures + 1))
fi

# Test 4: Missing filename
./bms_diag > test_output.txt 2>&1
code=$?

if [ "$code" -eq 2 ] &&
   grep -q 'Usage:' test_output.txt; then
    echo "PASS: missing argument"
else
    echo "FAIL: missing argument (exit $code)"
    failures=$((failures + 1))
fi

# Test 5: Two valid frames
cat valid.bin valid.bin > two_frames.bin

if ./bms_diag two_frames.bin > test_output.txt 2>&1 &&
   grep -q 'Frame 2' test_output.txt &&
   grep -q 'Frames processed: 2' test_output.txt; then
    echo "PASS: two valid frames"
else
    echo "FAIL: two valid frames"
    failures=$((failures + 1))
fi

# Test 6: Incomplete second frame
printf '\xFF' > extra_byte.bin
cat valid.bin extra_byte.bin > trailing.bin

./bms_diag trailing.bin > test_output.txt 2>&1
code=$?

if [ "$code" -eq 1 ] &&
   grep -q 'Invalid frame length' test_output.txt; then
    echo "PASS: trailing incomplete frame"
else
    echo "FAIL: trailing incomplete frame"
    failures=$((failures + 1))
fi

# Test 7: Unsupported version
printf '\x4E\x52\x02\x00\x34\x12\xAB\xCD' > bad_version.bin

./bms_diag bad_version.bin > test_output.txt 2>&1
code=$?

if [ "$code" -eq 1 ] &&
   grep -q 'Unsupported version' test_output.txt; then
    echo "PASS: unsupported version"
else
    echo "FAIL: unsupported version"
    failures=$((failures + 1))
fi

# Test 8: Empty telemetry file
: > empty.bin

./bms_diag empty.bin > test_output.txt 2>&1
code=$?

if [ "$code" -eq 1 ] &&
   grep -q 'No telemetry frames found' test_output.txt; then
    echo "PASS: empty telemetry file"
else
    echo "FAIL: empty telemetry file"
    failures=$((failures + 1))
fi

# Remove temporary output.
rm -f test_output.txt

# Final test report.
echo "Failed tests: $failures"

if [ "$failures" -eq 0 ]; then
    exit 0
else
    exit 1
fi
