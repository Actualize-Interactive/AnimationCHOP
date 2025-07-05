# Point

A Point represents a time-value pair used for keyframe positions and handle coordinates.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `time` | `float` | Time coordinate |
| `value` | `float` | Value coordinate |
| `state` | `dict` | Serializable state dictionary |

## Constructor

#### `Point(time: float = 0, value: float = 0)`

Create a new Point with specified time and value coordinates.

## Methods

### State Management

#### `get_state() -> dict`
Get the point state as a dictionary containing time and value.

#### `set_state(state: dict) -> None`
Set the point state from a dictionary.

### Copy Operations

#### `__copy__() -> Point`
Create a shallow copy of the point.

#### `__deepcopy__(memo) -> Point`
Create a deep copy of the point.

## Comparison

Points support equality comparison:

```python
if point1 == point2:
    print("Points are identical")
```

## String Representation

Points have a readable string representation:

```python
print(point)  # Output: Point(time=10.0, value=50.0)
```

## Examples

```python
anim_chop = anim_chop('animation1')

# Create points
p1 = anim_chop.Point()  # Default: time=0, value=0
p2 = anim_chop.Point(30, 100)
p3 = anim_chop.Point(time=60, value=0)

# Modify coordinates
p1.time = 15.5
p1.value = 75.2

# Use in keyframe creation
keyframe = channel.create_keyframe(p2)

# Use as handles
in_handle = anim_chop.Point(25, 80)
out_handle = anim_chop.Point(35, 120)
keyframe = channel.create_keyframe(30, 100, in_handle, out_handle)

# State operations
state = p1.state  # {'time': 15.5, 'value': 75.2}
p_copy = anim_chop.Point()
p_copy.state = state

# Copy operations
p_shallow = p1.__copy__()
import copy
p_deep = copy.deepcopy(p1)

# Comparison
if p1 == p2:
    print("Points have same coordinates")
```
