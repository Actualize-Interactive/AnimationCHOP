# Channel

A Channel represents an individual animation curve containing keyframes. Channels support sequence operations for easy keyframe access and iteration.

A Channel is a **live handle**: it resolves to the operator's channel on every
access, so two Channel objects naming the same channel see each other's edits,
and one held past a `remove_channel()` raises `RuntimeError`. The keyframes it
returns are **detached copies** — see
[Keyframes are values, Channels are handles](README.md#keyframes-are-values-channels-are-handles).

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `name` | `str` | Channel name |
| `size` | `int` | Number of keyframes (alias for num_keyframes) |
| `num_keyframes` | `int` | Number of keyframes |
| `empty` | `bool` | True if channel has no keyframes |
| `start_time` | `float` | Time of first keyframe (or 0 if empty) |
| `end_time` | `float` | Time of last keyframe (or 0 if empty) |
| `length` | `float` | Duration from start to end time |
| `state` | `dict` | Serializable state dictionary |

## Methods

### Keyframe Creation

#### `create_keyframe(time: float, value: float) -> Keyframe`
Create a keyframe at specified time and value with default bezier/smooth settings.

#### `create_keyframe(time: float, value: float, function: Function, handle_mode: HandleMode) -> Keyframe`
Create a keyframe with specified interpolation and handle settings.

#### `create_keyframe(position: Point) -> Keyframe`
Create a keyframe from a Point object.

#### `create_keyframe(position: Point, function: Function, handle_mode: HandleMode) -> Keyframe`
Create a keyframe from a Point with specified settings.

#### `create_keyframe(time: float, value: float, in_handle: Point, out_handle: Point) -> Keyframe`
Create a keyframe with custom handle positions.

#### `create_keyframe(time: float, value: float, in_handle: Point, out_handle: Point, function: Function, handle_mode: HandleMode) -> Keyframe`
Create a keyframe with full control over all parameters.

#### `create_keyframe_from_state(state: dict) -> Keyframe`
Create a keyframe from a state dictionary.

#### `emplace_keyframe(keyframe: Keyframe) -> Keyframe`
Insert an existing keyframe into the channel.

### Keyframe Access

All accessors below return a **detached copy**. Setting a property on the
returned Keyframe changes the copy only; write it back with
`channel[index] = kf` to apply it.

#### `keyframe(index: int) -> Keyframe`
Get a copy of the keyframe at specified index.

#### `prev_keyframe(time: float) -> Keyframe | None`
Get a copy of the keyframe immediately before the specified time.

#### `next_keyframe(time: float) -> Keyframe | None`
Get a copy of the keyframe immediately after the specified time.

#### `closest_keyframe(time: float) -> Keyframe | None`
Get a copy of the keyframe closest to the specified time.

#### `delete_keyframe(index: int) -> None`
Remove keyframe at specified index.

### Keyframe Modification

#### `update_keyframe(index: int, keyframe: Keyframe) -> None`
Replace keyframe at index with new keyframe. Equivalent to
`channel[index] = keyframe`. The time is clamped between the neighbouring
keyframes — keyframes never reorder — and the neighbouring handles are re-solved
as needed.

#### `set_keyframe_time(index: int, time: float) -> None`
Set the time of keyframe at index.

#### `set_keyframe_value(index: int, value: float) -> None`
Set the value of keyframe at index.

#### `set_keyframe_position(index: int, position: Point) -> None`
Set the position of keyframe at index.

#### `set_keyframe_in_handle(index: int, handle: Point) -> None`
Set the in handle of keyframe at index.

#### `set_keyframe_out_handle(index: int, handle: Point) -> None`
Set the out handle of keyframe at index.

#### `set_keyframe_function(index: int, function: Function) -> None`
Set the interpolation function of keyframe at index.

#### `set_keyframe_handle_mode(index: int, handle_mode: HandleMode) -> None`
Set the handle mode of keyframe at index.

### Evaluation

#### `evaluate(time: float) -> float`
Evaluate the channel at a specific time.

Both range methods take an optional trailing `range_end`. It defaults to
`RangeEnd.EXCLUSIVE`: the range is half-open, `end_time` is **not** sampled, and
a span of n sample periods gives n values. That is the timeline reading — a
sample covers the interval that follows it, and the end of a range is an edge —
so looping a curve or joining adjacent ranges does not repeat a value at the
seam. It is also what the operators' own output uses.

Pass `RangeEnd.INCLUSIVE` when samples are points on the curve rather than spans
of time: plotting, building a lookup table, numeric integration. Without it the
last point falls one step short of the end.

```python
ch.evaluate_range_by_rate(0, 2, 60)                      # 120 values, 0 .. 1.983
ch.evaluate_range_by_rate(0, 2, 60, op.RangeEnd.INCLUSIVE)  # 121, the last at 2.0
```

#### `evaluate_range(start_time: float, end_time: float, num_samples: int, range_end: RangeEnd = RangeEnd.EXCLUSIVE) -> list[float]`
Evaluate the channel over a time range with specified number of samples.

#### `evaluate_range_by_rate(start_time: float, end_time: float, sample_rate: float, range_end: RangeEnd = RangeEnd.EXCLUSIVE) -> list[float]`
Evaluate the channel over a time range with specified sample rate.

A channel has no `num_samples`. It knows only the extent of its own keyframes,
which is an editing concept rather than the range a host samples over, so a
count taken from it would quietly answer about the wrong span. Use the
operator's [`num_samples`](AnimationCHOP.md), or TouchDesigner's own
`numSamples` on the cooked output.

### State Management

#### `get_state() -> dict`
Get the channel state as a dictionary.

#### `set_state(state: dict) -> None`
Set the channel state from a dictionary.

## Sequence Operations

Channels support Python sequence operations:

```python
anim_chop = anim_chop('animation1')
channel = anim_chop.get_channel("tx")

# Length
count = len(channel)

# Indexing -- returns a detached copy
first_keyframe = channel[0]
last_keyframe = channel[-1]

# Assignment -- writes a keyframe back, same as update_keyframe(index, kf)
kf = channel[0]
kf.value = 55.0
channel[0] = kf

# Iteration
for keyframe in channel:
    print(f"Time: {keyframe.time}, Value: {keyframe.value}")

# Membership testing
if keyframe in channel:
    print("Keyframe exists in channel")
```

Indices may be negative. `del channel[index]` is not supported — use
`delete_keyframe(index)`.

## Examples

```python
# Create and populate channel
channel = anim_chop.create_channel("position_x")

# Add keyframes with different methods
kf1 = channel.create_keyframe(0, 0)  # Simple time/value
kf2 = channel.create_keyframe(30, 100, anim_chop.Function.LINEAR, anim_chop.HandleMode.FLAT)
kf3 = channel.create_keyframe(anim_chop.Point(60, 0))  # From Point

# Custom handles
in_handle = anim_chop.Point(40, 80)
out_handle = anim_chop.Point(50, 20)
kf4 = channel.create_keyframe(45, 50, in_handle, out_handle)

# Modify existing keyframes
channel.set_keyframe_value(0, 10)  # Change first keyframe value
channel.set_keyframe_function(1, anim_chop.Function.BEZIER)

# Evaluate animation
current_value = channel.evaluate(15)  # Value at time 15
values = channel.evaluate_range(0, 60, 60)  # 60 samples over 60 seconds

# Find keyframes
prev_kf = channel.prev_keyframe(25)
next_kf = channel.next_keyframe(25)
closest_kf = channel.closest_keyframe(25)

# State operations
state = channel.state  # Save channel
channel.set_state(state)  # Restore channel
```
