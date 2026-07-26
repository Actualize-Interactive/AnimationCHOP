# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the major version is `0`, breaking changes may land in a minor release.

## [Unreleased]

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
  `TESTING.md`.
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

### Changed

- Updated to anim, which makes `Id`'s constructor private, returns references
  rather than pointers from its `Id` lookups, and moves rate-based sampling to a
  half-open range.
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
