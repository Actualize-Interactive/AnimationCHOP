# AnimationCHOP

[![CI](https://github.com/Actualize-Interactive/AnimationCHOP/actions/workflows/ci.yml/badge.svg)](https://github.com/Actualize-Interactive/AnimationCHOP/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)

Keyframe animation curves for TouchDesigner, as native C++ Custom Operators with
a full Python API.

Build curves from keyframes with cubic Bézier interpolation, drive them from
Python, and get them back as CHOP channels — without rebuilding an animation
system out of Pattern CHOPs and Lookup CHOPs every time.

The animation maths lives in [anim](https://github.com/Actualize-Interactive/anim),
a standalone C++20 library; these operators are the TouchDesigner binding around
it.

## What's included

| | |
| --- | --- |
| **AnimationCHOP** | Holds named animation channels and cooks them to CHOP samples. This is the operator you script against. |
| **AnimationViewCHOP** | Exposes an AnimationCHOP's internals as CHOP data — samples, keyframes, Bézier segments, channels — for building UI on top. |
| **Keyframer.tox** | A ready-made keyframe editor component built on both operators, with a GLSL-rendered curve view. |

## Features

- Named channels, each an independent curve; multiple channels per operator
- Per-keyframe interpolation: `Constant` (step), `Linear`, `Bezier`
- Bézier handle modes: `Flat`, `Smooth`, `Aligned`, `Free`, and the aligned
  variants `AlignStrict` / `AlignFlex` / `AlignAdjustable`
- Per-channel extrapolation before the first and after the last keyframe:
  `Hold`, `Repeat`, `Mirror`
- Output modes: a fixed time range, an auto range fitted to the content, an
  index driven by an input CHOP, or a single scrubbed sequence index
- A complete Python API — create and edit channels and keyframes, evaluate the
  curve at any time, and save or restore whole-animation state as a plain dict

## Requirements

- **TouchDesigner** — a build providing Custom Operator CHOP API version 10
  (common API version 2), i.e. one with node data persistence
  (`saveData`/`loadData`). Earlier builds will refuse to load the operators.
- **Windows or macOS.** Windows builds are x64.

## Installation

1. Download the archive for your platform from
   [Releases](https://github.com/Actualize-Interactive/AnimationCHOP/releases).
2. Copy the operator libraries into a `Plugins` folder beside your `.toe`:

   ```
   MyProject/
     MyProject.toe
     Plugins/
       AnimationCHOP.dll        (or .plugin on macOS)
       AnimationViewCHOP.dll
   ```

   TouchDesigner only loads Custom Operators from a `Plugins` folder next to the
   project file, or from the system-wide plugin folder.
3. Open the project. The first load of a new operator build shows a prompt
   asking you to trust it — approve it once.
4. Add an **AnimationCHOP** from the operator palette, or drop in the included
   `Keyframer.tox` for the full editor.

The release archive also contains an example project you can open directly.

## Quick start

Everything is driven from Python on the operator itself:

```python
n = op('animation1')

# Create a channel and key it
tx = n.create_channel('tx')
tx.create_keyframe(0, 0)
tx.create_keyframe(30, 100, n.Function.BEZIER, n.HandleMode.SMOOTH)

# Evaluate anywhere on the curve
print(tx.evaluate(15))

# Edit in place...
tx.set_keyframe_value(1, 250)

# ...or read, modify, write back
kf = tx[0]
kf.function = n.Function.LINEAR
tx[0] = kf

# Save and restore the whole animation
state = n.state
n.clear()
n.state = state
```

One thing to know before writing much against it: **`Channel` is a live handle,
but `Keyframe` and `Point` are values.** `tx[0]` gives you a detached copy, so
setting a property on it does not reach the channel until you write it back.
[The docs explain why](docs/README.md#keyframes-are-values-channels-are-handles) —
it is a consequence of how keyframes have to be re-solved against their
neighbours.

## Documentation

Full Python API reference under [`docs/`](docs/README.md):

- [AnimationCHOP](docs/AnimationCHOP.md) — the operator: channels, range, state
- [Channel](docs/Channel.md) — a single curve
- [Keyframe](docs/Keyframe.md) · [Point](docs/Point.md)
- [Function](docs/Function.md) · [HandleMode](docs/HandleMode.md)

## Building from source

```bash
git clone --recurse-submodules https://github.com/Actualize-Interactive/AnimationCHOP.git
cd AnimationCHOP
```

(If you already cloned without submodules: `git submodule update --init --recursive`.)

```powershell
# Windows
.\build.ps1
```

```bash
# macOS
./build.sh
```

Either way the built operators are copied into `td/Plugins/` so the example
project picks them up.

Windows vendors the CPython 3.11 headers and import libraries it needs, so there
is nothing to install. macOS builds against TouchDesigner's own Python framework
and expects TouchDesigner in `/Applications`.

## Testing

```powershell
cmake --workflow --preset dev     # configure, build, run the unit tests
```

The pytest suite runs the real binding sources against a fake TouchDesigner
context, so it needs no TouchDesigner install. There is also an integration
harness that drives a real project inside TouchDesigner. See
[TESTING.md](TESTING.md).

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

MIT — see [LICENSE](LICENSE).

The TouchDesigner SDK headers under `ext/td/` are Derivative Inc.'s and carry
their own terms; the vendored CPython headers are under the PSF License. See
[NOTICE](NOTICE) for the full third-party attribution.
