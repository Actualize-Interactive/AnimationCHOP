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

Two modules implement it:

- **`animation_test.py`** *is* the suite. `run_api_tests(anim_chop)` runs the
  binding suites (Point, Keyframe, enums, animation core, channel, advanced,
  error handling, state, state errors, state roundtrip) against the real
  operator. `setup_cook_test()` / `check_cook_test()` then do what only an
  in-TouchDesigner run can: configure the node's output parameters, let it cook,
  and check that the samples it emits match what the channels evaluate to.
- **`td_test_runner.py`** drives them. It runs the API suites, schedules the
  cook checks a few frames later (the node has to actually cook in between —
  that is what `run(..., delayFrames=)` is for; sleeping would block the very
  frames being waited on), and writes `results.json`.

The operator is passed in, so neither module needs to know where it lives.
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
| `animation_test` | `tests/td/animation_test.py` |

In each DAT, set the **File** parameter to the path above and use **Sync to
File** so TouchDesigner imports them by name.

**3. Add the bootstrap Execute DAT.**

This is the only place that needs to know where the operator is.

- Enable the Execute DAT's **Start** flag (the `onStart` callback).
- Paste:

  ```python
  def onStart():
      import td_test_runner
      td_test_runner.start(op('animation1'))   # point at your AnimationCHOP
      return
  ```

- **Save** `test.toe`.

(If you omit the argument, `start()` resolves the operator from the
`ANIMATIONCHOP_OP` environment variable that the host script sets from
`-OpName` / `--op`, defaulting to `animation1`.)

### Running the suite by hand

Useful while iterating — watch the textport for the PASS/FAIL lines:

```python
import animation_test
animation_test.run_api_tests(op('animation1'))
```

Or the whole thing including the cooked-output checks, which writes
`results.json` as well:

```python
import td_test_runner
td_test_runner.start(op('animation1'))
```

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
