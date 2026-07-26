"""In-TouchDesigner integration test driver.

Loaded as a module in TouchDesigner (a DAT under /local/modules, synced to this
file) and invoked from a bootstrap Execute DAT's onStart, which passes the
operators in -- the only place that needs to know where they live:

    def onStart():
        import td_test_runner
        td_test_runner.start(op('animation1'), op('animationview1'))
        return

The run is a sequence of steps. Some only need the Python API and run
immediately; the rest need the node to have cooked in a particular
configuration first, so they are split into a setup and a check a few frames
apart. run(..., delayFrames=) supplies the gap -- waiting on frames is the only
way to let a node cook, since sleeping would block the very frames being waited
on.

When the last step finishes, this module writes the results.json sentinel.
TouchDesigner is left running; the host script (run_td_tests.ps1 /
run_td_tests.sh) detects the sentinel and terminates it.

Environment variables (set by the host script):
    ANIMATIONCHOP_RESULTS  path to write results.json
                           (default: <project folder>/results.json)
    ANIMATIONCHOP_OP       name of the AnimationCHOP (default: animation1)
    ANIMATIONCHOP_VIEW_OP  name of the AnimationViewCHOP (default: animationview1)
"""

import os
import json
import traceback

import animation_chop_test
import animation_view_chop_test
from test_result import TestResult


# Frames to wait between configuring a node and reading what it cooked. One is
# enough in principle; a few gives parameter changes room to propagate on a
# loaded project without making the run feel slow.
COOK_DELAY_FRAMES = 10


# State carried across the frame gaps. Each deferred step comes back in as a
# fresh `import td_test_runner`, so module state is the handoff -- simpler than
# threading objects through run()'s args, and it does not depend on `me`
# resolving to this DAT.
_anim_chop = None
_view_chop = None
_result = None
_steps = []
_step_index = 0


def _results_path():
    default = os.path.join(project.folder, "results.json")  # noqa: F821 (TD global)
    return os.environ.get("ANIMATIONCHOP_RESULTS", default)


def _resolve(explicit, env_var, default_name):
    if explicit is not None:
        return explicit
    return op(os.environ.get(env_var, default_name))  # noqa: F821 (TD global)


def start(anim_chop=None, view_chop=None):
    """Run every suite against the operators and write the results sentinel.

    Pass the operators in from the bootstrap Execute DAT. If either is omitted
    its name is taken from the environment (see the module docstring).

    A missing AnimationViewCHOP is not fatal -- its suites are skipped and
    recorded as such, so an older test project still reports on everything else
    rather than failing wholesale.
    """
    global _anim_chop, _view_chop, _result, _steps, _step_index

    print("[td-test] start()")
    try:
        _anim_chop = _resolve(anim_chop, "ANIMATIONCHOP_OP", "animation1")
        if _anim_chop is None:
            _write_failure("No AnimationCHOP found. Check the Execute DAT wiring.")
            return

        _view_chop = _resolve(view_chop, "ANIMATIONCHOP_VIEW_OP", "animationview1")

        _result = TestResult()
        _steps = _build_steps()
        _step_index = 0
        _advance()
    except Exception as e:
        print(f"[td-test] start() failed: {e}")
        print(traceback.format_exc())
        _write_failure(f"{e}\n{traceback.format_exc()}")


def _build_steps():
    """The run, as a list of zero-argument callables.

    A step that configures a node returns nothing; the frame gap between every
    step is what lets the node cook before the next one reads it.
    """
    ac = animation_chop_test
    av = animation_view_chop_test

    steps = [
        lambda: ac.run_api_tests(_anim_chop, cleanup=True, result=_result),
        lambda: ac.setup_cook_test(_anim_chop),
        lambda: ac.check_cook_test(_anim_chop, _result),
    ]

    if _view_chop is None:
        print("[td-test] no AnimationViewCHOP found; skipping its suites")
        steps.append(lambda: _result.record_exception(
            "AnimationViewCHOP suites skipped",
            "No AnimationViewCHOP in the project (set ANIMATIONCHOP_VIEW_OP "
            "or pass it to start())"))
        return steps

    # Each view mode is a configure step followed by a check, since the node has
    # to cook in that mode before its output can be read.
    steps += [
        lambda: av.build_fixture(_anim_chop),
        lambda: av.configure_samples(_view_chop),
        lambda: av.check_samples_view(_anim_chop, _view_chop, _result),

        lambda: av.configure(_view_chop, 'keyframes'),
        lambda: av.check_keyframes_view(_anim_chop, _view_chop, _result),

        lambda: av.configure(_view_chop, 'segments'),
        lambda: av.check_segments_view(_anim_chop, _view_chop, _result),

        lambda: av.configure(_view_chop, 'channel'),
        lambda: av.check_channels_view(_anim_chop, _view_chop, _result),

        lambda: av.configure(_view_chop, 'animation'),
        lambda: av.check_animation_view(_anim_chop, _view_chop, _result),

        lambda: av.setup_empty_channel(_anim_chop, _view_chop),
        lambda: av.check_empty_channel(_anim_chop, _view_chop, _result),

        lambda: _anim_chop.clear(),
        lambda: av.check_empty_animation(_anim_chop, _view_chop, _result),
    ]
    return steps


def _advance():
    """Run the next step, then schedule the one after it a few frames later."""
    global _step_index

    if _step_index >= len(_steps):
        _finish()
        return

    step = _steps[_step_index]
    _step_index += 1

    try:
        step()
    except Exception as e:
        # One step failing should not strand the rest of the run, or the first
        # failure hides everything after it -- and without the sentinel the host
        # script can only report a timeout, which says nothing about why.
        print(f"[td-test] step {_step_index} failed: {e}")
        print(traceback.format_exc())
        if _result is not None:
            _result.record_exception(f"step {_step_index} aborted", e)

    run("import td_test_runner; td_test_runner._advance()",  # noqa: F821 (TD global)
        delayFrames=COOK_DELAY_FRAMES)


def _finish():
    try:
        _anim_chop.clear()
    except Exception:
        pass
    _result.print_summary()
    _write_results(_result)


def _summarize(records):
    """Collapse per-assertion records into one entry per suite.

    The run makes several hundred assertions; the host script prints a line per
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
