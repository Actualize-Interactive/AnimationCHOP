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
