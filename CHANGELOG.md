# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the major version is `0`, breaking changes may land in a minor release.

## [0.4.1] - 2026-09-08

### Fixed

- The macOS release did not load: TouchDesigner reported the plugins as
  corrupted. They were built on a runner without TouchDesigner, so CMake fell
  back to the runner's Homebrew Python 3.14 and linked against a path that
  exists on no user's Mac; they also required macOS 26 and Apple silicon, and
  carried only the linker's partial ad-hoc signature, which `codesign` rejects
  for the bundle. The plugins now link no Python library at all (the symbols
  resolve from TouchDesigner's own interpreter at load time, as for any
  CPython extension), build universal with a minimum of macOS 13.3, and are
  ad-hoc signed as complete bundles. The release workflow verifies all four
  properties before it will publish.
- The plugins are not notarized, so macOS still refuses a copy that carries the
  browser's quarantine flag, with the same "corrupted" report. The README's
  installation steps now include clearing the flag.
- `run_td_tests.sh` is executable.

## [0.4.0] - 2026-07-26

First release prepared for the public repository.

### Added

- Animations now persist inside the `.toe`. The operator implements
  TouchDesigner's `saveData()`/`loadData()`, so channels and keyframes survive a
  save and reload with no external file and no Python involved. This replaces
  saving state by hand from the component.
- `channel[index] = keyframe`, a synonym for `update_keyframe(index, keyframe)`.
  Keyframes come out of a channel as detached copies, so editing one through a
  `Keyframe` object is a read-modify-write; assignment makes that round trip
  discoverable from the subscript a user already reached for.
- A pytest suite covering the Python bindings, run against a compiled extension
  that builds the real binding sources against a fake TouchDesigner context. No
  TouchDesigner install needed, so it runs in CI.
- A Catch2 suite over the persistence codec, covering round-trip fidelity and
  its rejection of truncated, foreign and out-of-range data.
- A TouchDesigner integration harness (`run_td_tests.ps1` / `.sh`) that runs the
  suites inside a real project and gates its exit code on the result, including
  checks that the cooked CHOP output matches what the channels evaluate to. See
  `TESTING.md`. AnimationViewCHOP is covered there too, across all five view
  modes -- it has no Python API of its own, so integration is the only place it
  can be tested at all.
- `RangeEnd` is exposed to Python, as a trailing argument on
  `Channel.evaluate_range` and `Channel.evaluate_range_by_rate`. It defaults to
  `RangeEnd.EXCLUSIVE`; pass `RangeEnd.INCLUSIVE` when samples are points on the
  curve rather than spans of time, as when plotting or building a lookup table.
- `LICENSE`, `NOTICE`, `README.md` and `CONTRIBUTING.md`.

### Fixed

- **Breaking:** range mode sized its output as `end_time * sample_rate`, which
  ignored the range start. A node with `Range = [10, 70]` at 60 fps emitted 4200
  samples for a 60-second span. Both range and auto-range now take their count
  from `Animation::num_samples`, so the length and the data cannot drift.
  Output lengths change for any node whose range does not start at zero.
- **Breaking:** the range is now half-open. A span of n sample periods produces
  n samples spaced exactly `1/rate` apart, and the range end is no longer
  sampled — 30 seconds at 60 fps is 1800 samples, not 1801. Both operators now
  fill their output with `evaluate_range_by_rate()` rather than
  `evaluate_range()`, which spreads a count across a *closed* interval and so
  only lands on `1/rate` spacing for one particular count. A CHOP's samples are
  implicitly one period apart — the format stores no per-sample times — so the
  previous pairing skewed the whole channel whenever the two disagreed. This
  follows the same change in anim.
- AnimationViewCHOP's samples view was one sample short of its range in seconds
  mode, while its samples mode was correct. Both now clamp to at least one
  sample, so an inverted range cannot ask TouchDesigner for a negative count.
- Range mode assigned the start before the end unconditionally. Because the
  setters clamp against the current opposite bound, a node moving from `[0,30]`
  to `[50,70]` clamped the new start against the stale end and cooked `[30,70]`
  for a frame. It now settles in a single cook, matching what the Python
  `start_time`/`end_time` setters already did.
- The `Output Mode` parameter defaulted to `fullrange`, which is not one of its
  menu entries; it landed on the first entry by fallback.
- The `Sample Rate` parameter had its slider bounds swapped (minimum 120,
  maximum 30) on both operators.
- Holding a `Channel` past a `remove_channel()` unwound a C++ exception through
  the CPython boundary instead of raising. It now raises `RuntimeError`.
- NaN in the cooked output. TouchDesigner allocates a CHOP's sample buffer but
  does not initialise it, so any sample an operator does not write keeps
  whatever was in that memory -- which appears as NaN, apparently by design, so
  the omission is visible. Several paths wrote nothing at all: AnimationCHOP in
  Input mode with nothing connected, or with a mismatched input, set an error
  and returned; AnimationViewCHOP did the same with no source operator
  selected, which is the state a freshly created node is in. Those paths now
  write zeros, so a node that cannot produce data reports why rather than
  emitting NaN. The normal paths are unchanged and still write each sample
  exactly once. Closes #15.
- A channel with no keyframes crashed AnimationViewCHOP. The segment count was
  computed as `size() - 1` on an unsigned type, so an empty channel wrapped to
  `SIZE_MAX` and undercounted the segment table, which the fill loop then wrote
  past the end of. Creating a channel before keying it is ordinary, so this was
  reachable from the first thing a user does. Closes #16.

### Changed

- Updated to anim v0.4.0, which makes `Id`'s constructor private, returns
  references rather than pointers from its `Id` lookups, and moves sampling to a
  half-open range.
- **Breaking:** removed `Channel.num_samples(rate)`, following anim, which
  removed the method behind it in 0.4.0. A channel knows only the extent of its
  own keyframes -- an editing concept, not the range a host samples over -- so a
  count taken from it silently answered about the wrong span. Use the operator's
  `num_samples`, which counts over the configured range, or TouchDesigner's own
  `numSamples` on the cooked output.
- CI builds now run the test suites, and publish release archives that work on
  unzip: the operators in a `Plugins/` folder beside the example project,
  `Keyframer.tox`, the modules the project loads, and the licence files.

### Documentation

- Documented that `Channel` is a live handle while `Keyframe` and `Point` are
  values, in the type docstrings, on each keyframe accessor, and in `docs/`.
  Setting a property on a keyframe read from a channel changes only the copy;
  this is forced by anim, where a keyframe has no identity of its own and every
  edit has to be re-solved against its neighbours.

### Known limitations

- Restoring an animation from a `.toe` loses anim's cached pre-inheritance
  function and handle mode for each channel's last keyframe. The cache is
  private to the library, so the codec cannot persist it. The effect is only
  visible if a keyframe is appended after a reload: the formerly-last keyframe
  keeps its inherited function rather than reverting to the one it was created
  with.

<!--
## [0.3.1] and earlier

Released before this changelog was kept.
-->
