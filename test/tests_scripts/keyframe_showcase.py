"""
Keyframe Showcase Script for AnimationCHOP
This script demonstrates the new AnimationCHOP Python API by creating multiple channels
with different keyframe configurations, handle modes, and functions.
"""

def create_keyframe_showcase():
    """Create a comprehensive showcase using the new AnimationCHOP API"""
    
    # Get the AnimationCHOP node
    try:
        anim_chop = op('Animation1')
        print(f"Using AnimationCHOP node: {anim_chop}")
    except NameError:
        print("ERROR: This script must be run from within TouchDesigner")
        return
    
    # Clear all existing channels
    print("Clearing all existing channels...")
    anim_chop.clear()
    
    # Channel configurations
    channels_config = [
        {
            'name': 'position_x',
            'base_value': 0.0,
            'range': 2.0,
            'time_offset': 0.0,
            'functions': [anim_chop.Function.LINEAR, anim_chop.Function.BEZIER, anim_chop.Function.LINEAR, anim_chop.Function.BEZIER, anim_chop.Function.LINEAR],
            'handle_modes': [anim_chop.HandleMode.SMOOTH, anim_chop.HandleMode.FREE, anim_chop.HandleMode.ALIGNED, anim_chop.HandleMode.SMOOTH, anim_chop.HandleMode.FLAT]
        },
        {
            'name': 'position_y', 
            'base_value': 0.75,
            'range': 2.0,
            'time_offset': 0.2,
            'functions': [anim_chop.Function.BEZIER, anim_chop.Function.LINEAR, anim_chop.Function.BEZIER, anim_chop.Function.CONSTANT, anim_chop.Function.BEZIER],
            'handle_modes': [anim_chop.HandleMode.FREE, anim_chop.HandleMode.SMOOTH, anim_chop.HandleMode.ALIGN_STRICT, anim_chop.HandleMode.FLAT, anim_chop.HandleMode.FREE]
        },
        {
            'name': 'rotation',
            'base_value': 1.5,
            'range': 2.0, 
            'time_offset': 0.4,
            'functions': [anim_chop.Function.CONSTANT, anim_chop.Function.BEZIER, anim_chop.Function.LINEAR, anim_chop.Function.BEZIER, anim_chop.Function.CONSTANT],
            'handle_modes': [anim_chop.HandleMode.FLAT, anim_chop.HandleMode.ALIGNED, anim_chop.HandleMode.SMOOTH, anim_chop.HandleMode.ALIGN_FLEX, anim_chop.HandleMode.FLAT]
        },
        {
            'name': 'scale',
            'base_value': 2.25,
            'range': 2.0,
            'time_offset': 0.1,
            'functions': [anim_chop.Function.BEZIER, anim_chop.Function.BEZIER, anim_chop.Function.LINEAR, anim_chop.Function.BEZIER, anim_chop.Function.BEZIER],
            'handle_modes': [anim_chop.HandleMode.ALIGN_ADJUSTABLE, anim_chop.HandleMode.FREE, anim_chop.HandleMode.SMOOTH, anim_chop.HandleMode.FREE, anim_chop.HandleMode.SMOOTH]
        }
    ]
    
    print("Creating channels with new API...")
    print("=" * 80)
    
    created_channels = []
    
    for config in channels_config:
        try:
            # Create channel using new API
            channel = anim_chop.create_channel(config['name'])
            created_channels.append(channel)
            
            print(f"✓ Created channel: {config['name']}")
            
            # Create 5 keyframes for this channel
            for i in range(5):
                time = i * 1.0 + config['time_offset']  # Keyframes at offset intervals
                
                # Create a wave-like pattern within the range
                import math
                wave_factor = math.sin(i * math.pi / 2)  # Creates: 0, 1, 0, -1, 0 pattern
                value = config['base_value'] + (wave_factor * config['range'] / 2)
                
                # Get function and handle mode for this keyframe
                function = config['functions'][i]
                handle_mode = config['handle_modes'][i]
                
                # Create handle points for bezier curves
                if function == anim_chop.Function.BEZIER:
                    # Create meaningful handle positions
                    handle_offset = 0.3
                    in_handle = anim_chop.Point(time - handle_offset, value - 0.2 + (i % 2) * 0.4)
                    out_handle = anim_chop.Point(time + handle_offset, value + 0.2 - (i % 2) * 0.4)
                    
                    # Create keyframe with handles
                    keyframe = channel.create_keyframe(time, value, in_handle, out_handle, function, handle_mode)
                else:
                    # Create keyframe without custom handles for linear/constant
                    keyframe = channel.create_keyframe(time, value, function, handle_mode)
                
                print(f"    Keyframe {i}: time={time:.1f}, value={value:.2f}, function={function}, mode={handle_mode}")
            
            print(f"    Channel stats: {channel.num_keyframes} keyframes, start={channel.start_time:.1f}, end={channel.end_time:.1f}")
            print()
            
        except Exception as e:
            print(f"✗ Failed to create channel {config['name']}: {e}")
            import traceback
            traceback.print_exc()
    
    print("=" * 80)
    
    # Display animation summary
    print(f"ANIMATION SUMMARY:")
    print(f"  Total channels: {anim_chop.num_channels}")
    print(f"  Channel names: {anim_chop.channel_names}")
    print(f"  Animation start time: {anim_chop.start_time:.2f}")
    print(f"  Animation end time: {anim_chop.end_time:.2f}")
    print(f"  Animation length: {anim_chop.length:.2f}")
    print("=" * 80)
    
    # Test channel access methods
    print("TESTING CHANNEL ACCESS:")
    print("=" * 80)
    
    for i, channel in enumerate(created_channels):
        try:
            # Test different access methods
            by_index = anim_chop.get_channel(i)
            by_name = anim_chop.get_channel(channel.name)
            has_channel = anim_chop.has_channel(channel.name)
            
            print(f"Channel {i} ({channel.name}):")
            print(f"  Access by index: {by_index.name if by_index else 'None'}")
            print(f"  Access by name: {by_name.name if by_name else 'None'}")
            print(f"  Has channel: {has_channel}")
            
            # Test keyframe access
            if len(channel) > 0:
                first_kf = channel[0]
                last_kf = channel[-1]
                print(f"  First keyframe: time={first_kf.time:.1f}, value={first_kf.value:.2f}")
                print(f"  Last keyframe: time={last_kf.time:.1f}, value={last_kf.value:.2f}")
                
                # Test evaluation
                mid_time = (first_kf.time + last_kf.time) / 2
                mid_value = channel.evaluate(mid_time)
                print(f"  Mid evaluation (t={mid_time:.1f}): {mid_value:.2f}")
            
            print()
            
        except Exception as e:
            print(f"Error testing channel {i}: {e}")
    
    # Create TSV output for TableDAT
    print("EXPORTING TO TABLEDAT:")
    print("=" * 80)
    
    table_lines = []
    table_lines.append("channel\tindex\ttime\tvalue\tfunction\thandle_mode\tin_time\tin_value\tout_time\tout_value")
    
    for channel in created_channels:
        try:
            for i in range(len(channel)):
                kf = channel[i]
                
                # Get function and handle mode names (now they're readable!)
                func_name = f"Function.{kf.function}"  # Just use the integer value
                mode_name = f"HandleMode.{kf.handle_mode}"  # Just use the integer value
                
                row = f"{channel.name}\t{i}\t{kf.time:.2f}\t{kf.value:.3f}\t{func_name}\t{mode_name}\t{kf.in_handle.time:.2f}\t{kf.in_handle.value:.3f}\t{kf.out_handle.time:.2f}\t{kf.out_handle.value:.3f}"
                table_lines.append(row)
                
        except Exception as e:
            print(f"Error processing channel {channel.name}: {e}")
    
    # Output to TableDAT
    keyframes_tbl_str = "\n".join(table_lines)
    
    try:
        keyframes_dat = op('keyframes')
        
        # Temporarily disable callback if it exists
        callback_dat = None
        original_active = True
        try:
            callback_dat = op('on_keyframes_change')
            original_active = callback_dat.par.active.eval()
            callback_dat.par.active = False
            print("✓ Temporarily disabled keyframes change callback")
        except:
            print("! No callback DAT found")
        
        # Set table data
        keyframes_dat.text = keyframes_tbl_str
        print(f"✓ Exported {len(table_lines)-1} keyframe rows to TableDAT 'keyframes'")
        
        # Re-enable callback
        if callback_dat and original_active:
            callback_dat.par.active = True
            print("✓ Re-enabled keyframes change callback")
            
    except Exception as e:
        print(f"✗ Failed to export to TableDAT: {e}")
        
        # Ensure callback is re-enabled
        try:
            if callback_dat and original_active:
                callback_dat.par.active = True
        except:
            pass
    
    # Test evaluation ranges
    print("\nTESTING EVALUATION RANGES:")
    print("=" * 80)
    
    if created_channels:
        test_channel = created_channels[0]
        try:
            # Test range evaluation
            values = test_channel.evaluate_range(0.0, 4.0, 9)  # 9 samples from 0 to 4
            print(f"Range evaluation (0-4s, 9 samples): {[f'{v:.2f}' for v in values]}")
            
            # Test by sample rate
            values_rate = test_channel.evaluate_range_by_rate(0.0, 2.0, 2.0)  # 2Hz for 2 seconds
            print(f"Rate evaluation (0-2s, 2Hz): {[f'{v:.2f}' for v in values_rate]}")
            
            # Test num_samples calculation
            num_samples = test_channel.num_samples(30.0)  # 30Hz
            print(f"Number of samples at 30Hz: {num_samples}")
            
        except Exception as e:
            print(f"Error testing evaluation: {e}")
    
    print("=" * 80)
    print("SHOWCASE COMPLETE!")
    print(f"Created {len(created_channels)} channels with 5 keyframes each")
    print("Channels demonstrate different interpolation functions and handle modes")
    print("Values are offset and overlapping to create interesting animation curves")
    print("=" * 80)

# Run the showcase
if __name__ == "__main__":
    create_keyframe_showcase()
else:
    # When imported, also provide a way to run it
    create_keyframe_showcase()
