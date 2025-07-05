# HandleMode

HandleMode enumeration defines how keyframe handles behave, controlling tangent relationships and automatic positioning.

## Values

| Value | Name | Description |
|-------|------|-------------|
| `0` | `FLAT` | Handles are horizontal (flat tangents) |
| `1` | `SMOOTH` | Handles maintain smooth curves with automatic positioning |
| `2` | `ALIGNED` | Handles are aligned (opposite directions, same angle) |
| `3` | `FREE` | Handles can be positioned independently |
| `4` | `ALIGN_STRICT` | Strict alignment - handles locked to same angle and length |
| `5` | `ALIGN_FLEX` | Flexible alignment - same angle, independent lengths |
| `6` | `ALIGN_ADJUSTABLE` | Adjustable alignment - user can break/restore alignment |

## Access

Access HandleMode values through the AnimationCHOP object:

```python
# Via AnimationCHOP instance
flat_mode = anim_chop.HandleMode.FLAT                      # 0
smooth_mode = anim_chop.HandleMode.SMOOTH                  # 1
aligned_mode = anim_chop.HandleMode.ALIGNED                # 2
free_mode = anim_chop.HandleMode.FREE                      # 3
strict_mode = anim_chop.HandleMode.ALIGN_STRICT            # 4
flex_mode = anim_chop.HandleMode.ALIGN_FLEX                # 5
adjustable_mode = anim_chop.HandleMode.ALIGN_ADJUSTABLE    # 6
```

## Usage

Handle modes are used when creating or modifying keyframes with bezier interpolation:

```python
# Create keyframes with different handle modes
kf_flat = channel.create_keyframe(0, 0, anim_chop.Function.BEZIER, anim_chop.HandleMode.FLAT)
kf_smooth = channel.create_keyframe(30, 100, anim_chop.Function.BEZIER, anim_chop.HandleMode.SMOOTH)
kf_free = channel.create_keyframe(60, 0, anim_chop.Function.BEZIER, anim_chop.HandleMode.FREE)

# Modify existing keyframe handle mode
channel.set_keyframe_handle_mode(0, anim_chop.HandleMode.ALIGNED)

# In keyframe constructor
keyframe = anim_chop.Keyframe(
    time=30,
    value=50,
    function=anim_chop.Function.BEZIER,
    handle_mode=anim_chop.HandleMode.SMOOTH
)

# Direct assignment
keyframe.handle_mode = anim_chop.HandleMode.FREE
```

## Handle Mode Behavior

### FLAT
- Handles are always horizontal
- Creates step-like transitions with rounded corners
- Good for discrete value changes

### SMOOTH
- Automatic smooth curve calculation
- Handles adjust automatically for natural motion
- Best for organic, flowing animation

### ALIGNED
- In and out handles maintain same angle
- Can have different lengths
- Good for maintaining curve continuity

### FREE
- Complete independence of handle positions
- Maximum control for custom curves
- Useful for complex, non-standard interpolations

### ALIGN_STRICT
- Handles locked to same angle and length
- Most restrictive alignment mode
- Creates symmetric curves

### ALIGN_FLEX
- Same angle, independent lengths
- More flexible than ALIGN_STRICT
- Allows asymmetric but aligned curves

### ALIGN_ADJUSTABLE
- User can break and restore alignment
- Dynamic behavior based on user interaction
- Most flexible aligned mode

## Examples

```python
anim_chop = op('animation1')
# Compare handle modes
if keyframe.handle_mode == anim_chop.HandleMode.SMOOTH:
    print("Using automatic smooth handles")

# Mode-specific keyframe creation
# Flat handles for step-like motion
step_kf = channel.create_keyframe(10, 50, anim_chop.Function.BEZIER, anim_chop.HandleMode.FLAT)

# Smooth handles for natural motion
natural_kf = channel.create_keyframe(20, 100, anim_chop.Function.BEZIER, anim_chop.HandleMode.SMOOTH)

# Free handles for custom curves
custom_kf = channel.create_keyframe(30, 0, anim_chop.Function.BEZIER, anim_chop.HandleMode.FREE)

# Batch setting handle modes
for i in range(channel.num_keyframes):
    kf = channel.keyframe(i)
    if kf.function == anim_chop.Function.BEZIER:
        channel.set_keyframe_handle_mode(i, anim_chop.HandleMode.SMOOTH)

# Create keyframe with custom handles and free mode
in_handle = anim_chop.Point(25, 80)
out_handle = anim_chop.Point(35, 20)
custom_curve = channel.create_keyframe(
    30, 50, 
    in_handle, out_handle,
    anim_chop.Function.BEZIER, 
    anim_chop.HandleMode.FREE
)
```
