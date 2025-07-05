# AnimationCHOP

The main AnimationCHOP object contains multiple animation channels and provides the primary interface for animation data management.

## Properties

| Property | Type | Description | Access |
|----------|------|-------------|---------|
| `channels` | `list[Channel]` | List of all channels | Read-only |
| `channel_names` | `list[str]` | List of all channel names | Read-only |
| `num_channels` | `int` | Number of channels | Read-only |
| `start_time` | `float` | Animation start time | Read/Write |
| `end_time` | `float` | Animation end time | Read/Write |
| `length` | `float` | Animation length (end_time - start_time) | Read/Write |
| `num_samples` | `int` | Number of samples at current sample rate | Read-only |
| `state` | `dict` | Serializable state dictionary | Read/Write |

## Type Access

| Property | Type | Description |
|----------|------|-------------|
| `Point` | `type` | Point class for creating time-value pairs |
| `Keyframe` | `type` | Keyframe class for creating keyframes |
| `Function` | `enum` | Function enumeration for interpolation types |
| `HandleMode` | `enum` | HandleMode enumeration for handle behavior |

## Methods

### Channel Management

#### `create_channel(name: str) -> Channel`
Create a new channel with the specified name.

#### `create_channel(name: str, index: int) -> Channel`
Create a new channel at the specified index.

#### `get_channel(key: str | int) -> Channel`
Get a channel by name or index.

#### `has_channel(name: str) -> bool`
Check if a channel with the given name exists.

#### `remove_channel(key: str | int) -> None`
Remove a channel by name or index.

#### `clear() -> None`
Remove all channels.

### State Management

#### `get_state() -> dict`
Get the complete animation state as a dictionary.

#### `set_state(state: dict) -> None`
Set the animation state from a dictionary.

## Examples

```python
# Access the AnimationCHOP (assuming 'op' is your AnimationCHOP)
anim_chop = op('animation1')

# Create channels
tx_channel = anim_chop.create_channel("tx")
ty_channel = anim_chop.create_channel("ty")

# Set animation timing
anim_chop.start_time = 0
anim_chop.end_time = 60
anim_chop.length = 60  # Alternative way to set end_time

# Check channels
if anim_chop.has_channel("tx"):
    channel = anim_chop.get_channel("tx")

# Access all channels
for channel in anim_chop.channels:
    print(f"Channel: {channel.name}, Keyframes: {channel.num_keyframes}")

# Create types
point = anim_chop.Point(10, 50)
keyframe = anim_chop.Keyframe(time=10, value=50, function=anim_chop.Function.BEZIER)

# State management
state = anim_chop.state  # Save complete animation
anim_chop.clear()        # Clear all
anim_chop.state = state  # Restore animation
```
