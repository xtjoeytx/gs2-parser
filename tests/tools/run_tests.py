#!/usr/bin/env python3
"""
GS2 Parser Test Suite Runner
Comprehensive testing framework for ensuring bytecode consistency across refactoring.
"""

import os
import sys
import json
import hashlib
import subprocess
import argparse
import time
from pathlib import Path
from typing import Dict, List, Tuple, Optional
from dataclasses import dataclass, asdict

@dataclass
class TestResult:
    """Results from running a single test"""
    script_path: str
    success: bool
    compilation_time: float
    bytecode_hash: str
    bytecode_size: int
    error_message: str = ""
    warnings: List[str] = None

    def __post_init__(self):
        if self.warnings is None:
            self.warnings = []

@dataclass
class BaselineData:
    """Baseline data for comparison"""
    bytecode_hash: str
    bytecode_size: int
    compilation_success: bool
    expected_failure: bool = False
    error_message: str = ""
    metadata: Dict = None

    def __post_init__(self):
        if self.metadata is None:
            self.metadata = {}

class GS2TestRunner:
    """Main test runner class"""

    def __init__(self, project_root: Path, scripts_dir: Path = None,
                 baselines_dir: Path = None, output_dir: Path = None,
                 reports_dir: Path = None):
        self.project_root = project_root
        self.test_dir = project_root / "tests"

        # Allow overriding directories (for build directory mode)
        self.scripts_dir = scripts_dir or (self.test_dir / "scripts")
        self.baselines_dir = baselines_dir or (self.test_dir / "baselines")
        self.output_dir = output_dir or (self.test_dir / "outputs")
        self.reports_dir = reports_dir or (self.test_dir / "reports")
        self.tools_dir = self.test_dir / "tools"

        # Ensure directories exist
        self.baselines_dir.mkdir(parents=True, exist_ok=True)
        self.output_dir.mkdir(parents=True, exist_ok=True)
        self.reports_dir.mkdir(parents=True, exist_ok=True)

        # Find the compiler executable
        self.compiler_path = self._find_compiler()

    def _find_compiler(self) -> Path:
        """Find the GS2 compiler executable"""
        possible_paths = [
            self.project_root / "bin" / "gs2test",
            self.project_root / "build" / "gs2test",
            self.project_root / "build" / "Debug" / "gs2test",
            self.project_root / "build" / "Release" / "gs2test",
        ]

        for path in possible_paths:
            if path.exists():
                return path

        raise FileNotFoundError("Could not find gs2test compiler executable. Please build the project first.")

    def _get_script_files(self, category: Optional[str] = None) -> List[Path]:
        """Get all test script files, optionally filtered by category"""
        if category:
            category_dir = self.scripts_dir / category
            if not category_dir.exists():
                raise ValueError(f"Category '{category}' does not exist")
            return list(category_dir.glob("*.gs2"))

        # Get all .gs2 files recursively
        return list(self.scripts_dir.rglob("*.gs2"))

    def _compile_script(self, script_path: Path) -> Tuple[bool, bytes, str, float]:
        """Compile a single script and return results"""
        start_time = time.time()

        try:
            # Run the compiler
            result = subprocess.run(
                [str(self.compiler_path), str(script_path)],
                capture_output=True,
                text=True,
                timeout=30  # 30 second timeout
            )

            compilation_time = time.time() - start_time

            # Check if compilation was successful
            # Note: Compiler may return 0 even with errors, so also check for [ERROR] in output
            has_error_output = result.stdout and "[ERROR]" in result.stdout
            success = result.returncode == 0 and not has_error_output

            # Compiler outputs errors to stdout via printf, check there first
            error_message = result.stdout if result.stdout else (result.stderr if result.stderr else "")

            # Extract cleaner error message if [ERROR] is present
            if has_error_output:
                lines = result.stdout.split('\n')
                error_lines = [line for line in lines if "[ERROR]" in line]
                if error_lines:
                    error_message = error_lines[0].replace(" -> [ERROR] ", "")

            # The GS2 compiler outputs to <script>.gs2bc file
            bytecode = b""
            if success:
                # Expected output file is script_name.gs2bc (in same directory as script)
                temp_output_file = script_path.with_suffix(script_path.suffix + "bc")
                if temp_output_file.exists():
                    bytecode = temp_output_file.read_bytes()
                    temp_output_file.unlink()  # Clean up temporary file
                else:
                    # If no output file but success reported, that's unexpected
                    error_message = f"Compilation reported success but no output file found: {temp_output_file}"
                    success = False
            else:
                # Extract error from stdout since compiler prints there
                if result.stdout and "[ERROR]" in result.stdout:
                    lines = result.stdout.split('\n')
                    error_lines = [line for line in lines if "[ERROR]" in line]
                    if error_lines:
                        error_message = error_lines[0].replace(" -> [ERROR] ", "")

            return success, bytecode, error_message, compilation_time

        except subprocess.TimeoutExpired:
            compilation_time = time.time() - start_time
            return False, b"", "Compilation timeout", compilation_time
        except Exception as e:
            compilation_time = time.time() - start_time
            return False, b"", str(e), compilation_time

    def _get_baseline_path(self, script_path: Path) -> Path:
        """Get the baseline file path for a script"""
        relative_path = script_path.relative_to(self.scripts_dir)
        baseline_path = self.baselines_dir / relative_path.with_suffix(".json")
        baseline_path.parent.mkdir(parents=True, exist_ok=True)
        return baseline_path

    def _is_expected_failure(self, script_path: Path) -> bool:
        """Determine if this script is expected to fail compilation"""
        # Scripts in error_cases directory are expected to fail
        relative_path = script_path.relative_to(self.scripts_dir)
        return "error_cases" in relative_path.parts

    def _save_baseline(self, script_path: Path, success: bool, bytecode: bytes, error_message: str):
        """Save baseline data for a script"""
        baseline_path = self._get_baseline_path(script_path)
        expected_failure = self._is_expected_failure(script_path)

        baseline_data = BaselineData(
            bytecode_hash=hashlib.sha256(bytecode).hexdigest() if bytecode else "",
            bytecode_size=len(bytecode),
            compilation_success=success,
            expected_failure=expected_failure,
            error_message=error_message,
            metadata={
                "script_path": str(script_path.relative_to(self.scripts_dir)),
                "generated_at": time.strftime("%Y-%m-%d %H:%M:%S"),
                "compiler_version": self._get_compiler_version()
            }
        )

        with open(baseline_path, 'w') as f:
            json.dump(asdict(baseline_data), f, indent=2)

        # Also save raw bytecode for debugging
        if bytecode:
            bytecode_path = baseline_path.with_suffix(".bytecode")
            with open(bytecode_path, 'wb') as f:
                f.write(bytecode)

    def _load_baseline(self, script_path: Path) -> Optional[BaselineData]:
        """Load baseline data for a script"""
        baseline_path = self._get_baseline_path(script_path)

        if not baseline_path.exists():
            return None

        try:
            with open(baseline_path, 'r') as f:
                data = json.load(f)
            return BaselineData(**data)
        except Exception as e:
            print(f"Warning: Could not load baseline for {script_path}: {e}")
            return None

    def _get_compiler_version(self) -> str:
        """Get compiler version information"""
        try:
            result = subprocess.run(
                [str(self.compiler_path), "--version"],
                capture_output=True,
                text=True,
                timeout=5
            )
            if result.returncode == 0:
                return result.stdout.strip()
        except:
            pass

        # Fallback to file modification time
        stat = self.compiler_path.stat()
        return f"modified_{int(stat.st_mtime)}"

    def run_test(self, script_path: Path) -> TestResult:
        """Run a single test"""
        success, bytecode, error_message, compilation_time = self._compile_script(script_path)

        bytecode_hash = hashlib.sha256(bytecode).hexdigest() if bytecode else ""

        return TestResult(
            script_path=str(script_path.relative_to(self.scripts_dir)),
            success=success,
            compilation_time=compilation_time,
            bytecode_hash=bytecode_hash,
            bytecode_size=len(bytecode),
            error_message=error_message
        )

    def compare_with_baseline(self, test_result: TestResult, baseline: BaselineData) -> List[str]:
        """Compare test result with baseline and return list of differences"""
        differences = []

        # Check compilation success/failure consistency
        if test_result.success != baseline.compilation_success:
            if baseline.expected_failure:
                if test_result.success:
                    differences.append(f"Expected failure but compilation succeeded")
                else:
                    # This is expected - no difference to report
                    pass
            else:
                differences.append(f"Compilation success changed: {baseline.compilation_success} -> {test_result.success}")

        # For successful compilations, compare bytecode
        if test_result.success and baseline.compilation_success:
            if test_result.bytecode_hash != baseline.bytecode_hash:
                differences.append(f"Bytecode hash changed: {baseline.bytecode_hash} -> {test_result.bytecode_hash}")

            if test_result.bytecode_size != baseline.bytecode_size:
                differences.append(f"Bytecode size changed: {baseline.bytecode_size} -> {test_result.bytecode_size}")

        # For expected failures, check if they still fail appropriately
        if baseline.expected_failure and test_result.success:
            differences.append(f"Script was expected to fail but compiled successfully")

        # For failures, optionally check error message consistency
        if not test_result.success and not baseline.compilation_success:
            # Both failed - this is often expected, especially for error_cases
            # We could check error message consistency, but it might be too strict
            pass

        return differences

    def run_all_tests(self, category: Optional[str] = None, update_baselines: bool = False) -> Dict:
        """Run all tests and return results"""
        script_files = self._get_script_files(category)

        if not script_files:
            raise ValueError("No test scripts found")

        print(f"Running {len(script_files)} tests...")

        results = {
            "summary": {
                "total_tests": len(script_files),
                "passed": 0,
                "failed": 0,
                "expected_failures": 0
            },
            "tests": [],
            "regressions": [],
            "new_tests": [],
            "execution_time": 0
        }

        start_time = time.time()

        for i, script_path in enumerate(script_files, 1):
            print(f"[{i}/{len(script_files)}] Testing {script_path.relative_to(self.scripts_dir)}...")

            # Run the test
            test_result = self.run_test(script_path)

            # Load baseline for comparison
            baseline = self._load_baseline(script_path)

            # Determine if this is an expected failure
            expected_failure = self._is_expected_failure(script_path)

            test_info = {
                "script": test_result.script_path,
                "success": test_result.success,
                "compilation_time": test_result.compilation_time,
                "bytecode_size": test_result.bytecode_size,
                "error_message": test_result.error_message,
                "has_baseline": baseline is not None,
                "expected_failure": expected_failure,
                "is_regression": False,
                "differences": []
            }

            if update_baselines or baseline is None:
                # Save new baseline only if compilation succeeded or it's an expected failure
                success, bytecode, error_message, _ = self._compile_script(script_path)

                # Only save baseline if test passed OR it's an expected failure
                should_save_baseline = success or expected_failure

                if should_save_baseline:
                    self._save_baseline(script_path, success, bytecode, error_message)
                    if baseline is None:
                        results["new_tests"].append(test_info)
                        print(f"  -> Created new baseline")
                    else:
                        print(f"  -> Updated baseline")
                elif baseline is None:
                    print(f"  -> Skipping baseline creation (test failed)")
            elif baseline:
                # Compare with baseline
                differences = self.compare_with_baseline(test_result, baseline)
                if differences:
                    test_info["is_regression"] = True
                    test_info["differences"] = differences
                    results["regressions"].append(test_info)
                    print(f"  -> REGRESSION: {', '.join(differences)}")

            # Update summary counts
            if expected_failure:
                if not test_result.success:
                    results["summary"]["expected_failures"] += 1
                    print(f"  -> EXPECTED FAILURE: {test_result.error_message[:50]}...")
                else:
                    results["summary"]["failed"] += 1
                    print(f"  -> UNEXPECTED SUCCESS: Expected failure but compiled successfully")
            else:
                if test_result.success:
                    results["summary"]["passed"] += 1
                    if not test_info["is_regression"]:
                        print(f"  -> PASS")
                else:
                    results["summary"]["failed"] += 1
                    if not test_info["is_regression"]:
                        print(f"  -> FAILED: {test_result.error_message}")

            results["tests"].append(test_info)

        results["execution_time"] = time.time() - start_time

        return results

    def generate_report(self, results: Dict, output_file: Optional[Path] = None) -> Path:
        """Generate detailed test report"""
        if output_file is None:
            timestamp = time.strftime("%Y%m%d_%H%M%S")
            output_file = self.reports_dir / f"test_report_{timestamp}.json"

        # Add metadata
        results["metadata"] = {
            "generated_at": time.strftime("%Y-%m-%d %H:%M:%S"),
            "compiler_path": str(self.compiler_path.relative_to(self.project_root)),
            "compiler_version": self._get_compiler_version()
        }

        with open(output_file, 'w') as f:
            json.dump(results, f, indent=2)

        return output_file

def main():
    parser = argparse.ArgumentParser(description="GS2 Parser Test Suite Runner")
    parser.add_argument("--category", help="Run tests only for specific category")
    parser.add_argument("--update-baselines", action="store_true",
                       help="Update baselines with current results")
    parser.add_argument("--project-root", type=Path, default=Path.cwd(),
                       help="Path to project root directory")
    parser.add_argument("--scripts-dir", type=Path,
                       help="Directory containing test scripts (default: PROJECT_ROOT/tests/scripts)")
    parser.add_argument("--baselines-dir", type=Path,
                       help="Directory for baseline files (default: PROJECT_ROOT/tests/baselines)")
    parser.add_argument("--output-dir", type=Path,
                       help="Directory for generated bytecode (default: PROJECT_ROOT/tests/outputs)")
    parser.add_argument("--reports-dir", type=Path,
                       help="Directory for test reports (default: PROJECT_ROOT/tests/reports)")
    parser.add_argument("--output-report", type=Path,
                       help="Output file for test report")
    parser.add_argument("--quiet", action="store_true",
                       help="Suppress verbose output")
    parser.add_argument("--show-timing", action="store_true",
                       help="Show detailed compilation timing for each test")

    args = parser.parse_args()

    try:
        runner = GS2TestRunner(args.project_root,
                              scripts_dir=args.scripts_dir,
                              baselines_dir=args.baselines_dir,
                              output_dir=args.output_dir,
                              reports_dir=args.reports_dir)

        # Run tests
        results = runner.run_all_tests(
            category=args.category,
            update_baselines=args.update_baselines
        )

        # Generate report
        report_file = runner.generate_report(results, args.output_report)

        # Extract summary for exit code logic
        summary = results["summary"]

        # Print summary (unless quiet)
        if not args.quiet:
            print(f"\n{'='*60}")
            print(f"Test Summary:")
            print(f"  Total Tests: {summary['total_tests']}")
            print(f"  Passed: {summary['passed']}")
            print(f"  Failed: {summary['failed']}")
            print(f"  Expected Failures: {summary['expected_failures']}")
            print(f"  Regressions: {len(results['regressions'])}")
            print(f"  New Tests: {len(results['new_tests'])}")
            print(f"  Execution Time: {results['execution_time']:.2f}s")
            print(f"  Report saved to: {report_file}")

            if args.show_timing:
                print(f"\nCompilation Timing (microseconds):")
                total_compilation_time = 0
                for test in results["tests"]:
                    compilation_time_us = int(test["compilation_time"] * 1_000_000)
                    total_compilation_time += test["compilation_time"]
                    status = "✓" if test["success"] else "✗"
                    print(f"  {status} {test['script']}: {compilation_time_us:,} μs")

                total_compilation_us = int(total_compilation_time * 1_000_000)
                print(f"  Total compilation time: {total_compilation_us:,} μs")

            if results["new_tests"]:
                print(f"\nNew tests added:")
                for new_test in results["new_tests"]:
                    status = "✓" if new_test["success"] else "✗"
                    print(f"  {status} {new_test['script']}")

            if results["regressions"]:
                print(f"\nRegressions detected:")
                for regression in results["regressions"]:
                    print(f"  {regression['script']}: {', '.join(regression['differences'])}")

        # Exit with appropriate code
        exit_code = 0
        if len(results["regressions"]) > 0:
            exit_code = 1
        elif summary["failed"] > 0:
            exit_code = 2

        sys.exit(exit_code)

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(3)

if __name__ == "__main__":
    main()