# Day 1 — File Copy Debugging

## Initial defect
The original program requested 128 bytes from write()
even when read() returned fewer bytes.

## Corrections
- Validated argc
- Checked open() for -1
- Distinguished read errors from EOF
- Implemented a loop for partial writes
- Handled EINTR
- Checked write() and close() errors

## Testing
Eight local tests passed on WSL2.

## Lessons learned
Explain why the buffer offset must be updated
after each successful write().
