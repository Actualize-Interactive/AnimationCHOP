# AnimationCHOP Testing

Tests live under `tests/`:

- `tests/cpp/` — Catch2 over the `.toe` persistence codec (no TouchDesigner needed).
- `tests/python/` — pytest against a compiled test extension (no TouchDesigner needed).
- `tests/td/` — the TouchDesigner project, the in-network test modules, and the
  local integration harness (`run_td_tests.ps1` / `run_td_tests.sh`).

> Note: TouchDesigner cannot run in cloud CI (it needs a license and a GPU), so
> CI only builds the operators and runs the `tests/cpp` and `tests/python`
> suites. The `tests/td` integration test runs against a local TouchDesigner
> install.

Curve behaviour is not tested here — that belongs to the `anim` submodule, which
carries its own Catch2 suite. What these cover is the binding and TouchDesigner
glue built on top of it.

## Unit tests (no TouchDesigner)

```powershell
# Configure + build + run all unit tests in one command:
cmake --workflow --preset dev

# Re-run just the unit tests after a change:
ctest --preset dev
```

CMake auto-detects the uv Python 3.11 (no paths to pass), and the suite runs
through `uv run`, so pytest is fetched automatically — nothing to install first.
(Without uv, install `tests/python/requirements.txt` into the interpreter and
ctest will call `pytest` there instead.)

`tests/cpp` covers `src/animation_codec.cpp`, the binary format the operator
writes into the `.toe`. Those bytes are persisted user data, so the suite checks
both round-trip fidelity — including that a decoded curve evaluates identically
to the one that was saved — and that a truncated, foreign or out-of-range blob
is rejected outright rather than half-loaded. Catch2 is fetched at configure
time; nothing to install.

`tests/python` builds a small CPython extension (`animationchop`) that compiles
the *real* operator sources against a fake `PY_Context`, so the bindings are
exercised directly rather than through a copy. Once it is built you can also run
pytest on its own:

```powershell
uv run --with pytest pytest tests/python
```

## Integration test (local TouchDesigner)

`run_td_tests.ps1` is a local pre-release gate. Run the one script and wait for
pass/fail — it compiles the operators, copies them into `tests/td/Plugins/`,
launches TouchDesigner with `tests/td/test.toe`, runs the suites *inside*
TouchDesigner, writes a `results.json` sentinel, then parses the results,
terminates TouchDesigner, and exits non-zero if anything failed.

```powershell
.\run_td_tests.ps1                 # build + run + report
.\run_td_tests.ps1 -NoBuild        # reuse the already-built operators
.\run_td_tests.ps1 -OpName animation1 -TimeoutSec 180
```

```bash
./run_td_tests.sh                  # macOS
./run_td_tests.sh --no-build --op animation1
```

Four modules implement it, one per operator plus a driver and a shared harness:

- **`animation_chop_test.py`** — the AnimationCHOP suite. `run_api_tests()`
  covers the bindings (Point, Keyframe, enums, animation core, channel,
  advanced, error handling, state, state errors, state roundtrip) against the
  real operator. `setup_cook_test()` / `check_cook_test()` then do what only an
  in-TouchDesigner run can: configure the node's output parameters, let it cook,
  and check that the samples it emits match what the channels evaluate to.
- **`animation_view_chop_test.py`** — the AnimationViewCHOP suite. This operator
  has no Python API of its own, so *everything* about it is integration-only:
  the suite steps through all five view modes (samples, keyframes, segments,
  channels, animation), checking each publishes its documented channels and that
  the values match the source animation. It also covers empty and
  single-keyframe channels, which have no segments.
- **`td_test_runner.py`** drives them as a list of steps, a few frames apart. A
  step that reconfigures a node is followed by one that reads what it cooked —
  the gap is what lets the cook happen. That is what `run(..., delayFrames=)` is
  for; sleeping would block the very frames being waited on. It writes
  `results.json` when the last step finishes.
- **`test_result.py`** — the assertion harness both suites share. One
  `TestResult` is threaded through the whole run, so the summary and
  `results.json` cover everything rather than one module.

The operators are passed in, so no module needs to know where they live.
TouchDesigner is left running; the host script terminates it once the sentinel
appears.

### One-time wiring

`tests/td/test.toe` is not in the repo — create it once and save it. The steps:

**1. Create the project and the operator.**

- Make a new project and save it as `tests/td/test.toe`.
- Build first (`.\build.ps1`) so `tests/td/Plugins/` exists — TouchDesigner only
  loads Custom Operators from a `Plugins/` folder beside the `.toe`, and CMake's
  post-build step puts them there.
- Add an **AnimationCHOP** and name it `animation1`.
- Add an **AnimationViewCHOP** and name it `animationview1`, with its
  **Animation / Animation View CHOP** parameter pointing at `animation1`.
  (Without one, its suites are skipped and recorded as skipped; everything else
  still runs.)
- **First load after a (re)build:** TouchDesigner shows a modal asking you to
  approve/trust the newly built Custom Operator. Click to approve. This is
  interactive, so the first integration run after a rebuild may need a manual
  click.

**2. Add the test modules as DATs under `/local/modules`.**

Each is a Text DAT synced to its file on disk, so the repo stays the source of
truth:

| Module DAT (`/local/modules/…`) | Synced to file |
| --- | --- |
| `td_test_runner` | `tests/td/td_test_runner.py` |
| `test_result` | `tests/td/test_result.py` |
| `animation_chop_test` | `tests/td/animation_chop_test.py` |
| `animation_view_chop_test` | `tests/td/animation_view_chop_test.py` |

In each DAT, set the **File** parameter to the path above and use **Sync to
File** so TouchDesigner imports them by name. The DAT names must match the file
names, since the modules import each other by name.

**3. Add the bootstrap Execute DAT.**

This is the only place that needs to know where the operators are.

- Enable the Execute DAT's **Start** flag (the `onStart` callback).
- Paste:

  ```python
  def onStart():
      import td_test_runner
      td_test_runner.start(op('animation1'), op('animationview1'))
      return
  ```

- **Save** `test.toe`.

(Omit either argument and `start()` resolves it from the `ANIMATIONCHOP_OP` /
`ANIMATIONCHOP_VIEW_OP` environment variables the host script sets, defaulting
to `animation1` and `animationview1`.)

### Running the suite by hand

Useful while iterating — watch the textport for the PASS/FAIL lines:

```python
import animation_chop_test
animation_chop_test.run_api_tests(op('animation1'))
```

Or the whole thing — both operators, every view mode, the cooked-output checks —
which writes `results.json` as well:

```python
import td_test_runner
td_test_runner.start(op('animation1'), op('animationview1'))
```

Note that the full run spans many frames by design, so it finishes a second or
two after you invoke it, not immediately.

### Troubleshooting

**"no results.json after N s"** — the runner never finished. Most likely:
`test.toe` has no bootstrap Execute DAT (or its Start flag is off); TouchDesigner
is sitting on the trust modal for a freshly rebuilt operator; or the project was
saved with the timeline paused, in which case the frame-delayed cook checks never
fire. Open the project by hand once and check the textport — if the API suites
printed but nothing else did, it is the paused-timeline case.

**The operator does not appear in the palette** — `tests/td/Plugins/` is missing
or empty. Run `.\build.ps1`.

**Cook checks fail but the API suites pass** — the bindings are fine and the
node's output path is not. `check_cook_test()` prints the worst sample delta,
which distinguishes "wrong values" from "wrong sample count".
