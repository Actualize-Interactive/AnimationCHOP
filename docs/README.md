# AnimationCHOP Python Bindings Documentation

This document provides comprehensive reference for the Python bindings available in the AnimationCHOP TouchDesigner plugin.

## Core Types

- [**AnimationCHOP**](AnimationCHOP.md) - Main animation container with multiple channels
- [**Channel**](Channel.md) - Individual animation channel containing keyframes
- [**Keyframe**](Keyframe.md) - Animation keyframe with position, handles, and interpolation settings
- [**Point**](Point.md) - Time-value pair for positions and handles

## Enumerations

- [**Function**](Function.md) - Interpolation functions for keyframes
- [**HandleMode**](HandleMode.md) - Handle behavior modes for keyframes

## Quick Start

```python
# Create a channel
channel = op.create_channel("tx")

# Add keyframes
keyframe1 = channel.create_keyframe(0, 0)
keyframe2 = channel.create_keyframe(30, 100, op.Function.BEZIER, op.HandleMode.SMOOTH)

# Evaluate animation
value = channel.evaluate(15)  # Get value at time 15

# Access keyframes
for kf in channel:
    print(f"Time: {kf.time}, Value: {kf.value}")
```

## Keyframes are values, Channels are handles

This is the one piece of the API worth reading before you write against it.

**`Channel` is a live handle.** It resolves to the operator's channel on every
access, so two `Channel` objects naming the same channel see each other's edits,
and a `Channel` held past a `remove_channel()` raises `RuntimeError` rather than
reading freed memory.

**`Keyframe` and `Point` are values.** `channel.keyframe(i)`, `channel[i]`,
iteration, `prev_keyframe`, `next_keyframe` and `closest_keyframe` all hand back
a *detached copy*. Setting a property on that copy changes the copy alone:

```python
kf = channel[0]
kf.value = 55.0          # updates the copy
channel[0].value         # still the original value -- the channel is untouched
```

That is not an oversight. A keyframe has no identity of its own: they live in a
time-ordered list, and any edit has to clamp the keyframe's time between its
neighbours, re-solve their handles and invalidate the channel's evaluation
cache. Only the channel can do that, so only the channel exposes mutators.

(Because times are clamped rather than re-sorted, keyframes never reorder — an
index stays valid until a keyframe is created or deleted.)

There are two ways to edit a keyframe, and both are fine:

```python
# In place -- best when you are changing one thing.
channel.set_keyframe_value(0, 55.0)

# Round trip -- best when you are changing several at once.
kf = channel[0]
kf.value = 55.0
kf.function = op.Function.LINEAR
channel[0] = kf                      # same as channel.update_keyframe(0, kf)
```

The same applies one level down: `kf.in_handle` is a copy too, so
`kf.in_handle.time = 5` does nothing. Assign a whole `Point` instead —
`kf.in_handle = op.Point(5, 20)`.

The `Keyframe` property setters are not dead weight — they are how you build a
keyframe to hand *to* a channel, via `create_keyframe`, `emplace_keyframe` or
the round trip above.

## State Management

All objects support serializable state dictionaries for saving/loading:

```python
# Save state
state = op.state
channel_state = channel.state
keyframe_state = keyframe.state

# Restore state
op.state = state
channel.state = channel_state
keyframe.state = keyframe_state
```
