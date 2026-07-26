"""Shared assertion harness for the in-TouchDesigner test modules.

One TestResult is threaded through every suite in a run, so the summary and the
results.json the runner writes cover the whole run rather than one module.
"""

import inspect
import sys


class TestResult:
    """Collects assertions, printing as it goes and recording for results.json.

    Assertions are grouped into suites so a run of several hundred can be
    reported as a handful of lines plus the individual failures.
    """

    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.errors = []
        self.records = []
        self.suite = "general"

    def begin_suite(self, name):
        self.suite = name

    def _record(self, passed, message, detail=""):
        self.records.append({
            "suite": self.suite,
            "name": message,
            "passed": bool(passed),
            "detail": detail,
        })

    def _get_caller_line(self):
        """Get the line number of the calling test function"""
        frame = inspect.currentframe()
        try:
            # Go up the stack to find the test function call
            # currentframe -> assert_* method -> test function
            caller_frame = frame.f_back.f_back
            return caller_frame.f_lineno
        finally:
            del frame
    
    def _get_exception_line(self):
        """Get the line number where the current exception occurred"""
        try:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            if exc_traceback:
                # Walk up the traceback to find the line in our test file
                tb = exc_traceback
                while tb.tb_next:
                    tb = tb.tb_next
                return tb.tb_lineno
        except:
            # If anything goes wrong getting line number, just return None
            pass
        return None
    
    def assert_true(self, condition, message):
        line_no = self._get_caller_line()
        if condition:
            self.passed += 1
            self._record(True, message)
            print(f"✓ PASS: {message}")
        else:
            self.failed += 1
            error_msg = f"✗ FAIL: {message} (line {line_no})"
            self._record(False, message, f"line {line_no}")
            print(error_msg)
            self.errors.append(error_msg)

    def assert_false(self, condition, message):
        self.assert_true(not condition, message)

    def assert_equal(self, expected, actual, message):
        line_no = self._get_caller_line()
        if expected == actual:
            self.passed += 1
            self._record(True, message)
            print(f"✓ PASS: {message} (expected: {expected}, got: {actual})")
        else:
            self.failed += 1
            error_msg = f"✗ FAIL: {message} (expected: {expected}, got: {actual}) (line {line_no})"
            self._record(False, message,
                         f"expected {expected}, got {actual} (line {line_no})")
            print(error_msg)
            self.errors.append(error_msg)

    def assert_not_none(self, value, message):
        self.assert_true(value is not None, message)

    def assert_none(self, value, message):
        self.assert_true(value is None, message)

    def assert_near(self, expected, actual, tolerance, message):
        line_no = self._get_caller_line()
        if abs(expected - actual) <= tolerance:
            self.passed += 1
            self._record(True, message)
            print(f"✓ PASS: {message} (expected: {expected}, got: {actual}, tolerance: {tolerance})")
        else:
            self.failed += 1
            error_msg = f"✗ FAIL: {message} (expected: {expected}, got: {actual}, tolerance: {tolerance}) (line {line_no})"
            self._record(False, message,
                         f"expected {expected}, got {actual} "
                         f"(tolerance {tolerance}, line {line_no})")
            print(error_msg)
            self.errors.append(error_msg)

    def record_exception(self, test_name, exception):
        """Record an exception with its actual line number"""
        try:
            line_no = self._get_exception_line()
            if line_no:
                error_msg = f"✗ FAIL: {test_name} - {exception} (line {line_no})"
            else:
                error_msg = f"✗ FAIL: {test_name} - {exception}"
        except:
            error_msg = f"✗ FAIL: {test_name} - {exception}"

        self.failed += 1
        self._record(False, test_name, str(exception))
        print(error_msg)
        self.errors.append(error_msg)

    def print_summary(self):
        total = self.passed + self.failed
        print(f"\n{'='*50}")
        print(f"TEST SUMMARY")
        print(f"{'='*50}")
        print(f"Total tests: {total}")
        print(f"Passed: {self.passed}")
        print(f"Failed: {self.failed}")
        print(f"Success rate: {(self.passed/total*100) if total > 0 else 0:.1f}%")
        
        if self.errors:
            print(f"\nFAILED TESTS:")
            for error in self.errors:
                print(f"  {error}")


