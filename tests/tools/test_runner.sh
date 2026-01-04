#!/bin/bash
# Convenience wrapper for run_tests.py
# Simply forwards all arguments to the Python test runner

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
exec python3 "$SCRIPT_DIR/run_tests.py" "$@"