# Function

Function enumeration defines the interpolation methods available for keyframes.

## Values

| Value | Name | Description |
|-------|------|-------------|
| `0` | `CONSTANT` | Constant/step interpolation - value stays constant until next keyframe |
| `1` | `LINEAR` | Linear interpolation - straight line between keyframes |
| `2` | `BEZIER` | Bezier interpolation - smooth curve using handle positions |

## Access

Access Function values through the AnimationCHOP object:

```python
# Via AnimationCHOP instance
constant_func = anim_chop.Function.CONSTANT    # 0
linear_func = anim_chop.Function.LINEAR        # 1
bezier_func = anim_chop.Function.BEZIER        # 2
```

## Usage

Functions are used when creating or modifying keyframes:

```python
# Create keyframes with different functions
kf_constant = channel.create_keyframe(0, 0, anim_chop.Function.CONSTANT, anim_chop.HandleMode.FLAT)
kf_linear = channel.create_keyframe(30, 100, anim_chop.Function.LINEAR, anim_chop.HandleMode.FLAT) 
kf_bezier = channel.create_keyframe(60, 0, anim_chop.Function.BEZIER, anim_chop.HandleMode.SMOOTH)

# Modify existing keyframe function
channel.set_keyframe_function(0, anim_chop.Function.BEZIER)

# In keyframe constructor
keyframe = anim_chop.Keyframe(
    time=30, 
    value=50, 
    function=anim_chop.Function.LINEAR
)

# Direct assignment
keyframe.function = anim_chop.Function.CONSTANT
```

## Examples

```python
anim_chop = op('animation1')

# Compare functions
if keyframe.function == anim_chop.Function.BEZIER:
    print("Using bezier interpolation")

# Function-specific keyframe creation
# Constant - good for discrete values
step_kf = channel.create_keyframe(10, 1, anim_chop.Function.CONSTANT)

# Linear - good for mechanical motion  
linear_kf = channel.create_keyframe(20, 50, anim_chop.Function.LINEAR)

# Bezier - good for organic motion
smooth_kf = channel.create_keyframe(30, 100, anim_chop.Function.BEZIER, anim_chop.HandleMode.SMOOTH)

# Batch setting functions
for i in range(channel.num_keyframes):
    channel.set_keyframe_function(i, anim_chop.Function.BEZIER)
```
