# Contributing to AnimationCHOP

Thanks for your interest. Bug reports and pull requests are welcome.

## Where things live

| Path | |
| --- | --- |
| `src/` | The two operators. `animation_chop.cpp` is the scriptable one; `animation_view_chop.cpp` exposes its internals as CHOP data. |
| `src/py_anim_bindings/` | The CPython bindings for anim's types — `Channel`, `Keyframe`, `Point`, and the enums. |
| `ext/anim/` | The animation curve library, as a submodule. Curve maths belongs [there](https://github.com/Actualize-Interactive/anim), not here. |
| `ext/td/` | Derivative's Custom Operator SDK headers. Do not edit; they are vendored verbatim. |
| `ext/Python/` | Vendored CPython 3.11 headers and import libraries for the Windows build. |
| `td/` | The example project, `Keyframer.tox`, and its Python modules and shaders. |
| `tests/python/` | pytest against a compiled test extension. No TouchDesigner needed. |
| `tests/td/` | The in-TouchDesigner integration suite and its runner. |
| `docs/` | The Python API reference. |

**Curve behaviour changes go to `anim`.** If a fix is about how a curve
interpolates, how handles are solved, or how a channel extrapolates, it belongs
in the library, which has its own Catch2 suite. This repository is the
TouchDesigner binding.

## Building

```bash
git clone --recurse-submodules https://github.com/Actualize-Interactive/AnimationCHOP.git
cd AnimationCHOP
```

```powershell
.\build.ps1      # Windows
```

```bash
./build.sh       # macOS
```

## Tests

Run these before opening a pull request:

```powershell
cmake --workflow --preset dev
```

That configures, builds both operators plus the test extension, and runs the
pytest suite. It needs no TouchDesigner install — the extension compiles the
real binding sources against a fake `PY_Context`.

If your change touches the operators' cooking, output sizing, or parameters,
also run the integration suite against a real TouchDesigner:

```powershell
.\run_td_tests.ps1
```

See [TESTING.md](TESTING.md), including the one-time project wiring it needs.

## Adding tests

New Python API surface needs a test in `tests/python/`. Prefer that suite: it is
fast, runs in CI, and needs no license or GPU. Reach for `tests/td/` only for
behaviour that genuinely requires TouchDesigner — cooking, output channels,
parameters.

If you find surprising-but-intended behaviour, pin it with a test that says why
in its docstring, rather than leaving it undocumented. There are several already
(keyframes being detached copies, the range being independent of channel
content); they exist so the next person does not read them as bugs.

## Style

Match the file you are editing — this codebase predates any linter and is not
uniformly formatted. Tabs and 4-space indentation both appear; follow the
surrounding block.

Comments should explain *why*, not restate the code. If something looks odd but
is deliberate — an invariant, a TouchDesigner API constraint, an ordering that
matters — say so, since that is what a reader cannot recover from the code.

## Pull requests

- Branch from `main`.
- Keep a pull request to one concern.
- Say what you changed and why. If it changes behaviour, say what a user would
  notice.
- Note whether you ran the TouchDesigner integration suite, since CI cannot.

## Reporting bugs

Include:

- Your TouchDesigner version and platform.
- The AnimationCHOP version (release tag, or commit if built from source).
- A minimal repro — ideally a few lines of Python against a fresh
  AnimationCHOP, or a small `.toe`.
- What you expected and what happened.

Curve-shape and interpolation issues are usually better filed against
[anim](https://github.com/Actualize-Interactive/anim); if you are not sure, file
here and it can be moved.

## License

By contributing you agree that your contributions are licensed under the MIT
License, the same as the rest of this repository. Note that `ext/td/` and
`ext/Python/` carry their own third-party terms — see [NOTICE](NOTICE).
