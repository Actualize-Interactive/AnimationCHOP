"""In-TouchDesigner integration test driver for AnimationCHOP.

Loaded as a module in TouchDesigner (a DAT under /local/modules, synced to this
file) and invoked from a bootstrap Execute DAT's onStart, which passes the
AnimationCHOP operator in -- the only place that needs to know where it lives:

    def onStart():
        import td_test_runner
        td_test_runner.start(op('animation1'))
        return

start() runs the binding suite immediately, then schedules the cooked-output
checks a few frames later so the node has actually cooked in between, and writes
the results.json sentinel. TouchDesigner is left running; the host script
(run_td_tests.ps1 / run_td_tests.sh) detects the sentinel and terminates it.

Environment variables (set by the host script):
    ANIMATIONCHOP_RESULTS  path to write results.json
                           (default: <project folder>/results.json)
"""

import os
import json
import traceback

import animation_test


# The node has to cook between setup_cook_test() and check_cook_test(). One
# frame is enough in principle; a few gives the parameter changes room to
# propagate on a loaded project without making the run feel slow.
COOK_DELAY_FRAMES = 10


# Carried across the frame gap between start() and _finish(). The deferred call
# comes back in as a fresh `import td_test_runner`, so module state is the
# handoff -- simpler than threading objects through run()'s args, and it does not
# depend on `me` resolving to this DAT.
_pending_op = None
_pending_result = None


def _results_path():
    default = os.path.join(project.folder, "results.json")  # noqa: F821 (TD global)
    return os.environ.get("ANIMATIONCHOP_RESULTS", default)


def start(anim_chop=None):
    """Run the suites against anim_chop and write the results sentinel.

    Pass the operator in from the bootstrap Execute DAT. If it is omitted, the
    name is taken from ANIMATIONCHOP_OP (which the host script sets from
    -OpName / --op), falling back to 'animation1'.

    Any failure here still writes results.json -- if the run dies silently the
    host script can only report a timeout, which says nothing about why.
    """
    global _pending_op, _pending_result

    print("[td-test] start()")
    try:
        if anim_chop is None:
            name = os.environ.get("ANIMATIONCHOP_OP", "animation1")
            anim_chop = op(name)  # noqa: F821 (TD global)
            if anim_chop is None:
                _write_failure(f"No operator named '{name}' in the project.")
                return

        result = animation_test.run_api_tests(anim_chop, cleanup=True)
        if result is None:
            _write_failure("run_api_tests returned nothing -- no operator?")
            return

        animation_test.setup_cook_test(anim_chop)
        _pending_op = anim_chop
        _pending_result = result

        # run() is TouchDesigner's builtin frame-delayed scheduler. Waiting on
        # frames is the only way to let the node cook; sleeping would block the
        # very frames we are waiting for.
        run("import td_test_runner; td_test_runner._finish()",  # noqa: F821 (TD global)
            delayFrames=COOK_DELAY_FRAMES)
    except Exception as e:
        print(f"[td-test] start() failed: {e}")
        print(traceback.format_exc())
        _write_failure(f"{e}\n{traceback.format_exc()}")


def _finish():
    """Second half: the node has cooked, so check its output and write out."""
    anim_chop, result = _pending_op, _pending_result
    if result is None:
        _write_failure("_finish() ran with no pending result")
        return

    try:
        animation_test.check_cook_test(anim_chop, result)
        result.print_summary()
    except Exception as e:
        print(f"[td-test] cook checks failed: {e}")
        print(traceback.format_exc())
        result.record_exception("cook checks aborted", e)
    finally:
        try:
            anim_chop.clear()
        except Exception:
            pass
        _write_results(result)


def _summarize(records):
    """Collapse per-assertion records into one entry per suite.

    The suite runs several hundred assertions; the host script prints a line per
    suite plus every individual failure, which is the useful shape for a gate.
    """
    order = []
    suites = {}
    for r in records:
        name = r.get("suite", "general")
        if name not in suites:
            suites[name] = {"name": name, "passed": 0, "failed": 0}
            order.append(name)
        key = "passed" if r["passed"] else "failed"
        suites[name][key] += 1
    return [suites[n] for n in order]


def _write_results(result):
    records = result.records
    failures = [r for r in records if not r["passed"]]
    summary = {
        "results": [
            {
                "name": s["name"],
                "passed": s["failed"] == 0,
                "detail": f"{s['passed']} passed, {s['failed']} failed",
            }
            for s in _summarize(records)
        ],
        "failures": [
            {"suite": r["suite"], "name": r["name"], "detail": r["detail"]}
            for r in failures
        ],
        "passed": result.passed,
        "failed": result.failed,
        "success": result.failed == 0 and result.passed > 0,
    }
    _dump(summary)


def _write_failure(message):
    """Write a results file describing a run that never got to the assertions."""
    _dump({
        "results": [{"name": "runner", "passed": False, "detail": message}],
        "failures": [{"suite": "runner", "name": "startup", "detail": message}],
        "passed": 0,
        "failed": 1,
        "success": False,
    })


def _dump(summary):
    path = _results_path()
    with open(path, "w") as f:
        json.dump(summary, f, indent=2)
    print(f"[td-test] wrote {path}: "
          f"{summary['passed']} passed, {summary['failed']} failed")
