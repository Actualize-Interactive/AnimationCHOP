"""
Test suite for AnimationCHOP Python API
This script tests all the exposed Python functions for channel and keyframe management.
Run this in TouchDesigner with an AnimationCHOP node.
"""

import traceback

class TestResult:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.errors = []
    
    def assert_true(self, condition, message):
        if condition:
            self.passed += 1
            print(f"✓ PASS: {message}")
        else:
            self.failed += 1
            error_msg = f"✗ FAIL: {message}"
            print(error_msg)
            self.errors.append(error_msg)
    
    def assert_false(self, condition, message):
        self.assert_true(not condition, message)
    
    def assert_equal(self, expected, actual, message):
        if expected == actual:
            self.passed += 1
            print(f"✓ PASS: {message} (expected: {expected}, got: {actual})")
        else:
            self.failed += 1
            error_msg = f"✗ FAIL: {message} (expected: {expected}, got: {actual})"
            print(error_msg)
            self.errors.append(error_msg)
    
    def assert_not_none(self, value, message):
        self.assert_true(value is not None, message)
    
    def assert_none(self, value, message):
        self.assert_true(value is None, message)
    
    def print_summary(self):
        total = self.passed + self.failed
        print(f"\n{'='*50}")
        print(f"TEST SUMMARY")
        print(f"{'='*50}")
        print(f"Total tests: {total}")
        print(f"Passed: {self.passed}")
        print(f"Failed: {self.failed}")
        print(f"Success rate: {(self.passed/total*100) if total > 0 else 0:.1f}%")
        
        if self.errors:
            print(f"\nFAILED TESTS:")
            for error in self.errors:
                print(f"  {error}")

def test_channel_management(anim_chop, result):
    """Test channel creation, retrieval, and removal"""
    print("\n--- Testing Channel Management ---")
    
    # Test creating channels
    channel1 = anim_chop.create_channel("test_channel_1")
    result.assert_not_none(channel1, "create_channel returns channel object")
    
    channel2 = anim_chop.create_channel("test_channel_2")
    result.assert_not_none(channel2, "create second channel")
    
    # Test duplicate channel creation (should fail)
    try:
        duplicate = anim_chop.create_channel("test_channel_1")
        result.assert_none(duplicate, "duplicate channel creation should return None")
    except Exception as e:
        print(f"Expected error for duplicate channel: {e}")
    
    # Test getting channel names
    names = anim_chop.get_channel_names()
    result.assert_true("test_channel_1" in names, "channel names contains test_channel_1")
    result.assert_true("test_channel_2" in names, "channel names contains test_channel_2")
    result.assert_true(len(names) >= 2, "at least 2 channels exist")
    
    # Test getting channels by name
    retrieved_channel = anim_chop.get_channel("test_channel_1")
    result.assert_not_none(retrieved_channel, "get_channel by name returns channel")
    
    # Test getting channels by index
    retrieved_by_index = anim_chop.get_channel(0)
    result.assert_not_none(retrieved_by_index, "get_channel by index returns channel")
    
    # Test getting non-existent channel
    try:
        nonexistent = anim_chop.get_channel("nonexistent_channel")
        result.assert_none(nonexistent, "get_channel for nonexistent channel should return None")
    except Exception as e:
        print(f"Expected error for nonexistent channel: {e}")
    
    # Test removing channels
    anim_chop.remove_channel("test_channel_2")
    names_after_removal = anim_chop.get_channel_names()
    result.assert_false("test_channel_2" in names_after_removal, "removed channel not in names list")

def test_keyframe_operations_by_time(anim_chop, result):
    """Test keyframe operations using time-based methods"""
    print("\n--- Testing Keyframe Operations (Time-based) ---")
    
    # Create a test channel
    channel = anim_chop.create_channel("keyframe_test_channel")
    result.assert_not_none(channel, "created keyframe test channel")
    
    # Test setting keyframes at specific times
    success1 = anim_chop.set_keyframe_at_time("keyframe_test_channel", 0.0, 1.0)
    result.assert_true(success1, "set_keyframe_at_time at time 0.0")
    
    success2 = anim_chop.set_keyframe_at_time("keyframe_test_channel", 1.0, 2.0)
    result.assert_true(success2, "set_keyframe_at_time at time 1.0")
    
    # Test setting keyframe with tangent handles
    success3 = anim_chop.set_keyframe_at_time("keyframe_test_channel", 2.0, 3.0, 0, -0.1, 2.8, 0.1, 3.2)
    result.assert_true(success3, "set_keyframe_at_time with tangent handles")
    
    # Test checking if keyframes exist
    has_keyframe_0 = anim_chop.has_keyframe_at_time("keyframe_test_channel", 0.0)
    result.assert_true(has_keyframe_0, "has_keyframe_at_time returns True for existing keyframe")
    
    has_keyframe_nonexistent = anim_chop.has_keyframe_at_time("keyframe_test_channel", 0.5)
    result.assert_false(has_keyframe_nonexistent, "has_keyframe_at_time returns False for non-existing keyframe")
    
    # Test getting keyframes at time
    keyframe_at_0 = anim_chop.get_keyframe_at_time("keyframe_test_channel", 0.0)
    result.assert_not_none(keyframe_at_0, "get_keyframe_at_time returns keyframe object")
    
    keyframe_nonexistent = anim_chop.get_keyframe_at_time("keyframe_test_channel", 0.5)
    result.assert_none(keyframe_nonexistent, "get_keyframe_at_time returns None for non-existing keyframe")
    
    # Test removing keyframes by time
    success_remove = anim_chop.remove_keyframe_at_time("keyframe_test_channel", 1.0)
    result.assert_true(success_remove, "remove_keyframe_at_time succeeds")
    
    has_keyframe_after_removal = anim_chop.has_keyframe_at_time("keyframe_test_channel", 1.0)
    result.assert_false(has_keyframe_after_removal, "keyframe removed successfully")

def test_keyframe_operations_by_index(anim_chop, result):
    """Test keyframe operations using index-based methods"""
    print("\n--- Testing Keyframe Operations (Index-based) ---")
    
    # Create a fresh test channel with some keyframes
    channel = anim_chop.create_channel("index_test_channel")
    anim_chop.set_keyframe_at_time("index_test_channel", 0.0, 1.0)
    anim_chop.set_keyframe_at_time("index_test_channel", 1.0, 2.0)
    anim_chop.set_keyframe_at_time("index_test_channel", 2.0, 3.0)
    
    # Use debug function to inspect channel state
    try:
        debug_info = anim_chop.debug_channel("index_test_channel")
        print(f"Debug info: {debug_info}")
        
        # Verify the debug info shows we have keyframes
        if debug_info.get('keyframe_count', 0) == 3:
            print("✓ Channel correctly reports 3 keyframes")
        else:
            print(f"✗ Expected 3 keyframes, got {debug_info.get('keyframe_count', 0)}")
            
    except Exception as e:
        print(f"Debug function error: {e}")
    
    # Test has_keyframe by index
    print("\nTesting has_keyframe by index:")
    for i in range(4):  # Test indices 0, 1, 2, 3
        try:
            has_kf = anim_chop.has_keyframe("index_test_channel", i)
            if i < 3:
                result.assert_true(has_kf, f"has_keyframe by index {i} returns True")
            else:
                result.assert_false(has_kf, f"has_keyframe by index {i} returns False for out-of-range")
        except Exception as e:
            result.failed += 1
            result.errors.append(f"has_keyframe by index {i} failed: {e}")
    
    # Test getting keyframes by index
    print("\nTesting get_keyframe by index:")
    for i in range(3):
        try:
            keyframe = anim_chop.get_keyframe("index_test_channel", i)
            result.assert_not_none(keyframe, f"get_keyframe by index {i} returns keyframe")
            if keyframe:
                print(f"  Keyframe {i}: {keyframe}")
        except Exception as e:
            result.failed += 1
            result.errors.append(f"get_keyframe by index {i} failed: {e}")
    
    # Test invalid index access
    try:
        keyframe = anim_chop.get_keyframe("index_test_channel", 10)
        result.assert_none(keyframe, "get_keyframe with invalid index should return None")
    except Exception as e:
        # This should raise an exception, which is expected
        result.passed += 1
        print(f"✓ get_keyframe with invalid index properly raised exception: {e}")
    
    # Test keyframe modification by index
    print("\nTesting keyframe modification by index:")
    try:
        # Modify the first keyframe (index 0)
        anim_chop.set_keyframe("index_test_channel", 0, 5.0)
        result.passed += 1
        print("✓ set_keyframe by index 0 completed successfully")
    except Exception as e:
        result.failed += 1
        result.errors.append(f"set_keyframe by index failed: {e}")
    
    # Test removing keyframe by index (remove the last one to avoid index shifting issues)
    print("\nTesting keyframe removal by index:")
    try:
        anim_chop.remove_keyframe("index_test_channel", 2)  # Remove the last keyframe
        result.passed += 1
        print("✓ remove_keyframe by index 2 completed successfully")
        
        # Verify it was removed
        try:
            debug_info_after = anim_chop.debug_channel("index_test_channel")
            if debug_info_after.get('keyframe_count', 0) == 2:
                result.passed += 1
                print("✓ Keyframe count correctly reduced to 2 after removal")
            else:
                result.failed += 1
                result.errors.append(f"Expected 2 keyframes after removal, got {debug_info_after.get('keyframe_count', 0)}")
        except Exception as e:
            print(f"Could not verify keyframe removal: {e}")
            
    except Exception as e:
        result.failed += 1
        result.errors.append(f"remove_keyframe by index failed: {e}")

def test_batch_keyframe_operations(anim_chop, result):
    """Test batch keyframe operations"""
    print("\n--- Testing Batch Keyframe Operations ---")
    
    # Create a test channel
    channel = anim_chop.create_channel("batch_test_channel")
    
    # Test setting multiple keyframes at once (time-based)
    keyframes_data = [
        {
            'time': 0.0,
            'value': 1.0,
            'mode': 0,
            'in_tangent_time': -0.1,
            'in_tangent_value': 0.9,
            'out_tangent_time': 0.1,
            'out_tangent_value': 1.1
        },
        {
            'time': 1.0,
            'value': 2.0,
            'mode': 0,
            'in_tangent_time': 0.9,
            'in_tangent_value': 1.9,
            'out_tangent_time': 1.1,
            'out_tangent_value': 2.1
        },
        {
            'time': 2.0,
            'value': 3.0,
            'mode': 0
        }
    ]
    
    try:
        anim_chop.set_keyframes_at_time("batch_test_channel", keyframes_data)
        print("✓ set_keyframes_at_time completed")
        result.passed += 1
    except Exception as e:
        result.failed += 1
        result.errors.append(f"set_keyframes_at_time failed: {e}")
    
    # Test setting multiple keyframes by index
    index_keyframes_data = [
        {
            'index': 0,
            'value': 5.0,
            'mode': 0,
            'in_tangent_time': -0.1,
            'in_tangent_value': 4.9,
            'out_tangent_time': 0.1,
            'out_tangent_value': 5.1
        },
        {
            'index': 1,
            'value': 6.0,
            'mode': 0
        }
    ]
    
    try:
        anim_chop.set_keyframes("batch_test_channel", index_keyframes_data)
        print("✓ set_keyframes by index completed")
        result.passed += 1
    except Exception as e:
        result.failed += 1
        result.errors.append(f"set_keyframes by index failed: {e}")
    
    # Test removing multiple keyframes at time
    times_to_remove = [0.0, 2.0]
    try:
        success = anim_chop.remove_keyframes_at_time("batch_test_channel", times_to_remove)
        result.assert_true(success, "remove_keyframes_at_time returns success")
    except Exception as e:
        result.failed += 1
        result.errors.append(f"remove_keyframes_at_time failed: {e}")

def test_error_conditions(anim_chop, result):
    """Test error conditions and edge cases"""
    print("\n--- Testing Error Conditions ---")
    
    # Test operations on non-existent channels
    try:
        success = anim_chop.set_keyframe_at_time("nonexistent_channel", 0.0, 1.0)
        result.assert_false(success, "set_keyframe_at_time on nonexistent channel should fail")
    except Exception as e:
        print(f"Expected error for nonexistent channel: {e}")
        result.passed += 1
    
    # Test has_keyframe_at_time on nonexistent channel (should return False, not error)
    has_keyframe = anim_chop.has_keyframe_at_time("nonexistent_channel", 0.0)
    result.assert_false(has_keyframe, "has_keyframe_at_time on nonexistent channel should return False")
    
    # Test invalid index operations
    channel = anim_chop.create_channel("error_test_channel")
    anim_chop.set_keyframe_at_time("error_test_channel", 0.0, 1.0)
    
    try:
        keyframe = anim_chop.get_keyframe("error_test_channel", 10)  # Invalid index
        result.assert_none(keyframe, "get_keyframe with invalid index should fail")
    except Exception as e:
        print(f"Expected error for invalid index: {e}")
        result.passed += 1

def test_python_types_exposure(anim_chop, result):
    """Test that Python types are properly exposed"""
    print("\n--- Testing Python Types Exposure ---")
    # Only test types that are actually exposed by the API
    try:
        keyframe_type = anim_chop.Keyframe
        result.assert_not_none(keyframe_type, "Keyframe type is accessible")
    except AttributeError:
        result.assert_true(False, "Keyframe type is accessible")
    try:
        channel_type = anim_chop.Channel
        result.assert_not_none(channel_type, "Channel type is accessible")
    except AttributeError:
        result.assert_true(False, "Channel type is accessible")
    try:
        animation_type = anim_chop.Animation
        result.assert_not_none(animation_type, "Animation type is accessible")
    except AttributeError:
        result.assert_true(False, "Animation type is accessible")

def cleanup_test_channels(anim_chop):
    """Clean up any test channels that might exist"""
    print("\n--- Cleaning up test channels ---")
    
    test_channel_names = [
        "test_channel_1",
        "test_channel_2", 
        "keyframe_test_channel",
        "index_test_channel",
        "batch_test_channel",
        "error_test_channel"
    ]
    
    for name in test_channel_names:
        try:
            anim_chop.remove_channel(name)
            print(f"Cleaned up channel: {name}")
        except:
            pass  # Channel might not exist

def run_tests(cleanup=False):

    # Get the current operator (this should be called from the AnimationCHOP node)
    try:
        # In TouchDesigner, 'me' refers to the current operator
        anim_chop = op('Animationchop1')
        print(f"Running tests on node: {anim_chop}")
    except NameError:
        print("ERROR: This script must be run from within TouchDesigner")
        print("Use: op('your_animationchop_name').run_tests()")
        return
    
    try:
        anim_chop.clear_channels()  # Clear any existing channels
        print("Cleared existing channels in AnimationCHOP")
    except Exception as e:
        print(f"Failed to clear channels: {e}")
        return

    # Initialize test results
    result = TestResult()
    
    print("="*60)
    print("ANIMATIONCHOP PYTHON API TEST SUITE")
    print("="*60)
    

    try:
        # Run all test suites
        test_channel_management(anim_chop, result)
        test_keyframe_operations_by_time(anim_chop, result)
        test_keyframe_operations_by_index(anim_chop, result)
        test_batch_keyframe_operations(anim_chop, result)
        test_error_conditions(anim_chop, result)
        test_python_types_exposure(anim_chop, result)

    except Exception as e:
        print(f"\nUNEXPECTED ERROR: {e}")
        print(traceback.format_exc())
        result.failed += 1
        result.errors.append(f"Unexpected error: {e}")
    
    finally:
        if cleanup:
            anim_chop.clear_channels()
        else:
            print("\nSkipping cleanup. Test channels will remain in AnimationCHOP.")

    # Print final results
    result.print_summary()
    
    return result

run_tests()
