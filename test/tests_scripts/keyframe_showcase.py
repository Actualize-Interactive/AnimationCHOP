"""
Keyframe Showcase Script for AnimationCHOP
This script demonstrates all tangent modes by creating a channel with 20 keyframes,
each using different tangent modes and properties.
"""

def create_keyframe_showcase():
    """Create a comprehensive showcase of keyframes with different tangent modes"""
    
    # Get the AnimationCHOP node
    try:
        anim_chop = op('Animationchop1')
        print(f"Using AnimationCHOP node: {anim_chop}")
    except NameError:
        print("ERROR: This script must be run from within TouchDesigner")
        return
    
    # Clear all existing channels
    print("Clearing all existing channels...")
    anim_chop.clear_channels()
    
    # Create a new showcase channel
    print("Creating showcase channel...")
    channel = anim_chop.create_channel("keyframe_showcase")
    if not channel:
        print("ERROR: Failed to create showcase channel")
        return
    
    # Define tangent modes (matching the enum values from tangent_mode.hpp)
    tangent_modes = {
        0: "flat",
        1: "linear", 
        2: "stepped",
        3: "smoothAuto",
        4: "smoothManual",
        5: "broken"
    }
    
    print(f"Creating 20 keyframes with varying tangent modes...")
    print("=" * 80)
    
    # Create 20 keyframes with different properties
    for i in range(20):
        time = i * 0.5  # Keyframes at 0.0, 0.5, 1.0, 1.5, etc.
        value = 10.0 + 5.0 * (i % 4)  # Values cycling between 10, 15, 20, 25
        
        # Cycle through tangent modes
        mode = i % len(tangent_modes)
        
        # Vary tangent handle positions based on keyframe index
        in_tangent_time = time - 0.1 - (i % 3) * 0.05  # Varying in-handle time offset
        in_tangent_value = value - 1.0 + (i % 2) * 2.0  # Varying in-handle value offset
        out_tangent_time = time + 0.1 + (i % 3) * 0.05  # Varying out-handle time offset
        out_tangent_value = value + 1.0 - (i % 2) * 2.0  # Varying out-handle value offset
        
        # Create the keyframe
        success = anim_chop.set_keyframe_at_time(
            "keyframe_showcase", 
            time, 
            value, 
            mode,
            in_tangent_time, 
            in_tangent_value,
            out_tangent_time, 
            out_tangent_value
        )
        
        if success:
            print(f"✓ Created keyframe {i:2d}: time={time:4.1f}, value={value:4.1f}, mode={mode} ({tangent_modes[mode]})")
        else:
            print(f"✗ Failed to create keyframe {i}")
    
    print("=" * 80)
    
    # Get debug information about the channel
    try:
        debug_info = anim_chop.debug_channel("keyframe_showcase")
        print(f"Channel Debug Info:")
        print(f"  Name: {debug_info.get('name', 'Unknown')}")
        print(f"  Keyframe Count: {debug_info.get('keyframe_count', 0)}")
        print(f"  Is Empty: {debug_info.get('is_empty', True)}")
        print(f"  Start Time: {debug_info.get('start_time', 'None')}")
        print(f"  End Time: {debug_info.get('end_time', 'None')}")
        print("=" * 80)
    except Exception as e:
        print(f"Could not get debug info: {e}")
    
    # Retrieve and display all keyframes
    print("KEYFRAME DETAILS:")
    print("=" * 80)
    
    keyframe_count = debug_info.get('keyframe_count', 0) if 'debug_info' in locals() else 20
    
    # Initialize TSV table string with header
    table_lines = []
    table_lines.append("index\ttime\tvalue\tmode\tin_time\tin_val\tout_time\tout_val")

    # Print table header to console as well
    print(f"{'idx':<3} {'time':<6} {'value':<6} {'mode':<12} {'in_time':<8} {'in_val':<8} {'out_time':<8} {'out_val':<8}")
    print("-" * 80)
    
    for i in range(keyframe_count):
        try:
            # Get keyframe by index
            keyframe = anim_chop.get_keyframe("keyframe_showcase", i)
            if keyframe:
                # Extract keyframe properties (with fallbacks)
                try:
                    time = getattr(keyframe, 'time', 'N/A')
                    value = getattr(keyframe, 'value', 'N/A')
                    mode_num = getattr(keyframe, 'mode', 'N/A')
                    mode_name = tangent_modes.get(mode_num, f"Unk({mode_num})") if isinstance(mode_num, int) else 'N/A'
                    
                    # Handle BezierHandle objects
                    in_handle = getattr(keyframe, 'in_handle', None)
                    out_handle = getattr(keyframe, 'out_handle', None)

                    if in_handle and hasattr(in_handle, 'time') and hasattr(in_handle, 'value'):
                        in_time = f"{in_handle.time:.3f}"
                        in_val = f"{in_handle.value:.3f}"
                    else:
                        in_time = "N/A"
                        in_val = "N/A"
                    
                    if out_handle and hasattr(out_handle, 'time') and hasattr(out_handle, 'value'):
                        out_time = f"{out_handle.time:.3f}"
                        out_val = f"{out_handle.value:.3f}"
                    else:
                        out_time = "N/A"
                        out_val = "N/A"
                    
                    # Format the row for console
                    time_str = f"{time:.1f}" if isinstance(time, (int, float)) else str(time)
                    value_str = f"{value:.1f}" if isinstance(value, (int, float)) else str(value)
                    
                    console_row = f"{i:<3} {time_str:<6} {value_str:<6} {mode_name:<12} {in_time:<8} {in_val:<8} {out_time:<8} {out_val:<8}"
                    print(console_row)
                    
                    # Format the row for TSV (using actual values, not formatted strings)
                    tsv_time = time if isinstance(time, (int, float)) else time_str
                    tsv_value = value if isinstance(value, (int, float)) else value_str
                    
                    tsv_row = f"{i}\t{tsv_time}\t{tsv_value}\t{mode_name}\t{in_time}\t{in_val}\t{out_time}\t{out_val}"
                    table_lines.append(tsv_row)
                    
                except Exception as attr_e:
                    error_row_console = f"{i:<3} {'ERR':<6} {'ERR':<6} {'ERR':<12} {'ERR':<8} {'ERR':<8} {'ERR':<8} {'ERR':<8}  # Error: {attr_e}"
                    print(error_row_console)
                    
                    error_row_tsv = f"{i}\tERR\tERR\tERR\tERR\tERR\tERR\tERR"
                    table_lines.append(error_row_tsv)
            else:
                none_row_console = f"{i:<3} {'None':<6} {'None':<6} {'None':<12} {'None':<8} {'None':<8} {'None':<8} {'None':<8}"
                print(none_row_console)
                
                none_row_tsv = f"{i}\tNone\tNone\tNone\tNone\tNone\tNone\tNone"
                table_lines.append(none_row_tsv)
                
        except Exception as e:
            fail_row_console = f"{i:<3} {'FAIL':<6} {'FAIL':<6} {'FAIL':<12} {'FAIL':<8} {'FAIL':<8} {'FAIL':<8} {'FAIL':<8}  # Error: {e}"
            print(fail_row_console)
            
            fail_row_tsv = f"{i}\tFAIL\tFAIL\tFAIL\tFAIL\tFAIL\tFAIL\tFAIL"
            table_lines.append(fail_row_tsv)
    
    # Create the complete TSV string
    keyframes_tbl_str = "\n".join(table_lines)
    
    # Output to TableDAT
    try:
        keyframes_dat = op('keyframes')
        
        # Get the callback DAT and temporarily disable it
        try:
            on_keyframes_change_dat = op('on_keyframes_change')
            original_active_state = on_keyframes_change_dat.par.active.eval()
            on_keyframes_change_dat.par.active = False
            print("✓ Temporarily disabled keyframes change callback")
        except:
            on_keyframes_change_dat = None
            original_active_state = True
            print("! Could not find 'on_keyframes_change' DAT - proceeding without disabling callbacks")
        
        # Set the table data
        keyframes_dat.text = keyframes_tbl_str
        print(f"✓ Keyframe data exported to TableDAT 'keyframes' ({len(table_lines)-1} rows)")
        
        # Re-enable the callback DAT
        if on_keyframes_change_dat and original_active_state:
            on_keyframes_change_dat.par.active = True
            print("✓ Re-enabled keyframes change callback")
            
    except Exception as e:
        print(f"\n✗ Failed to export to TableDAT 'keyframes': {e}")
        
        # Make sure to re-enable callbacks even if there was an error
        try:
            if on_keyframes_change_dat and original_active_state:
                on_keyframes_change_dat.par.active = True
                print("✓ Re-enabled keyframes change callback after error")
        except:
            pass

    print("=" * 80)
    
    # Also demonstrate getting keyframes by time
    print("=" * 80)
    print("TESTING KEYFRAME ACCESS BY TIME:")
    print("=" * 80)
    
    test_times = [0.0, 1.0, 2.5, 5.0, 9.5]
    for test_time in test_times:
        try:
            has_keyframe = anim_chop.has_keyframe_at_time("keyframe_showcase", test_time)
            keyframe = anim_chop.get_keyframe_at_time("keyframe_showcase", test_time)
            
            print(f"Time {test_time:4.1f}: has_keyframe={has_keyframe}, keyframe={keyframe is not None}")
            if keyframe:
                print(f"  {keyframe}")
            print()
            
        except Exception as e:
            print(f"Error checking time {test_time}: {e}")
    
    # Summary
    print("=" * 80)
    print("SHOWCASE COMPLETE!")
    print(f"Created a channel '{debug_info.get('name', 'keyframe_showcase')}' with {keyframe_count} keyframes")
    print("Each keyframe demonstrates different tangent modes and handle positions.")
    print("You can now examine the channel in TouchDesigner's animation editor.")
    print("=" * 80)

# Run the showcase
if __name__ == "__main__":
    create_keyframe_showcase()
else:
    # When imported, also provide a way to run it
    create_keyframe_showcase()
