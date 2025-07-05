# Keyframe

A Keyframe represents a single animation key with position, handles, and interpolation settings.

## Properties

| Property | Type | Description |
|----------|------|-------------|
| `time` | `float` | Time position of the keyframe |
| `value` | `float` | Value of the keyframe |
| `in_handle` | `Point` | Incoming tangent handle position |
| `out_handle` | `Point` | Outgoing tangent handle position |
| `function` | `Function` | Interpolation function type |
| `handle_mode` | `HandleMode` | Handle behavior mode |
| `state` | `dict` | Serializable state dictionary |

## Constructor

#### `Keyframe(time: float = 0, value: float = 0, in_handle: Point = None, out_handle: Point = None, function: Function = Function.BEZIER, handle_mode: HandleMode = HandleMode.SMOOTH)`

Create a new keyframe with specified parameters. If handles are not provided, they default to positions relative to the keyframe time.

## Methods

### State Management

#### `get_state() -> dict`
Get the keyframe state as a dictionary containing position, handles, function, and handle_mode.

#### `set_state(state: dict) -> None`
Set the keyframe state from a dictionary.

### Copy Operations

#### `__copy__() -> Keyframe`
Create a shallow copy of the keyframe.

#### `__deepcopy__(memo) -> Keyframe`
Create a deep copy of the keyframe.

## Comparison

Keyframes support equality comparison:

```python
if keyframe1 == keyframe2:
    print("Keyframes are identical")
```

## String Representation

Keyframes have a readable string representation:

```python
print(keyframe)  # Output: Keyframe(time=10.0, value=50.0, function=2, handle_mode=1)
```

## Examples
anim_chop = anim_chop('animation1')

```python
# Create keyframes
kf1 = anim_chop.Keyframe()  # Default: time=0, value=0
kf2 = anim_chop.Keyframe(time=30, value=100)
kf3 = anim_chop.Keyframe(
    time=60, 
    value=0, 
    function=anim_chop.Function.LINEAR, 
    handle_mode=anim_chop.HandleMode.FLAT
)

# Custom handles
in_handle = anim_chop.Point(50, 80)
out_handle = anim_chop.Point(70, 20)
kf4 = anim_chop.Keyframe(
    time=60, 
    value=50, 
    in_handle=in_handle, 
    out_handle=out_handle,
    function=anim_chop.Function.BEZIER,
    handle_mode=anim_chop.HandleMode.FREE
)

# Modify properties
kf1.time = 15
kf1.value = 75
kf1.function = anim_chop.Function.BEZIER
kf1.handle_mode = anim_chop.HandleMode.SMOOTH

# Handle manipulation
kf1.in_handle = anim_chop.Point(10, 50)
kf1.out_handle = anim_chop.Point(20, 100)

# State operations
state = kf1.state  # Save keyframe state
kf_copy = anim_chop.Keyframe()
kf_copy.state = state  # Restore state

# Copy operations
kf_shallow = kf1.__copy__()
import copy
kf_deep = copy.deepcopy(kf1)
```
