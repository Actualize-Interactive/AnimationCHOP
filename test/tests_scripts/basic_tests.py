"""
Test suite for AnimationCHOP Python API
This script tests all the exposed Python functions for channel and keyframe management.
Run this in TouchDesigner with an AnimationCHOP node.
"""

import traceback
import inspect
import sys

class TestResult:
    def __init__(self):
        self.passed = 0
        self.failed = 0
        self.errors = []
    
    def _get_caller_line(self):
        """Get the line number of the calling test function"""
        frame = inspect.currentframe()
        try:
            # Go up the stack to find the test function call
            # currentframe -> assert_* method -> test function
            caller_frame = frame.f_back.f_back
            return caller_frame.f_lineno
        finally:
            del frame
    
    def _get_exception_line(self):
        """Get the line number where the current exception occurred"""
        try:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            if exc_traceback:
                # Walk up the traceback to find the line in our test file
                tb = exc_traceback
                while tb.tb_next:
                    tb = tb.tb_next
                return tb.tb_lineno
        except:
            # If anything goes wrong getting line number, just return None
            pass
        return None
    
    def assert_true(self, condition, message):
        line_no = self._get_caller_line()
        if condition:
            self.passed += 1
            print(f"✓ PASS: {message}")
        else:
            self.failed += 1
            error_msg = f"✗ FAIL: {message} (line {line_no})"
            print(error_msg)
            self.errors.append(error_msg)
    
    def assert_false(self, condition, message):
        self.assert_true(not condition, message)
    
    def assert_equal(self, expected, actual, message):
        line_no = self._get_caller_line()
        if expected == actual:
            self.passed += 1
            print(f"✓ PASS: {message} (expected: {expected}, got: {actual})")
        else:
            self.failed += 1
            error_msg = f"✗ FAIL: {message} (expected: {expected}, got: {actual}) (line {line_no})"
            print(error_msg)
            self.errors.append(error_msg)
    
    def assert_not_none(self, value, message):
        self.assert_true(value is not None, message)
    
    def assert_none(self, value, message):
        self.assert_true(value is None, message)
    
    def assert_near(self, expected, actual, tolerance, message):
        line_no = self._get_caller_line()
        if abs(expected - actual) <= tolerance:
            self.passed += 1
            print(f"✓ PASS: {message} (expected: {expected}, got: {actual}, tolerance: {tolerance})")
        else:
            self.failed += 1
            error_msg = f"✗ FAIL: {message} (expected: {expected}, got: {actual}, tolerance: {tolerance}) (line {line_no})"
            print(error_msg)
            self.errors.append(error_msg)
    
    def record_exception(self, test_name, exception):
        """Record an exception with its actual line number"""
        try:
            line_no = self._get_exception_line()
            if line_no:
                error_msg = f"✗ FAIL: {test_name} - {exception} (line {line_no})"
            else:
                error_msg = f"✗ FAIL: {test_name} - {exception}"
        except:
            error_msg = f"✗ FAIL: {test_name} - {exception}"
        
        self.failed += 1
        print(error_msg)
        self.errors.append(error_msg)
    
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


def test_point_api(anim_chop, result):
    """Test Point class functionality"""
    print("\n--- Testing Point API ---")
    
    try:
        # Test Point constructor
        point1 = anim_chop.Point()
        result.assert_equal(0.0, point1.time, "Point default constructor time")
        result.assert_equal(0.0, point1.value, "Point default constructor value")
        
        point2 = anim_chop.Point(1.5, 2.5)
        result.assert_equal(1.5, point2.time, "Point constructor with args - time")
        result.assert_equal(2.5, point2.value, "Point constructor with args - value")
        
        # Test Point property setters
        point2.time = 3.0
        point2.value = 4.0
        result.assert_equal(3.0, point2.time, "Point.time setter")
        result.assert_equal(4.0, point2.value, "Point.value setter")
        
        # Test Point string representation
        point_str = str(point2)
        result.assert_true("Point" in point_str, "Point string representation contains 'Point'")
        result.assert_true("3.0" in point_str, "Point string representation contains time")
        result.assert_true("4.0" in point_str, "Point string representation contains value")
        
        # Test Point equality
        point3 = anim_chop.Point(3.0, 4.0)
        result.assert_true(point2 == point3, "Point equality comparison")
        result.assert_false(point1 == point2, "Point inequality comparison")
        
    except Exception as e:
        result.record_exception("Point API error", e)


def test_keyframe_api(anim_chop, result):
    """Test Keyframe class functionality"""
    print("\n--- Testing Keyframe API ---")
    
    try:
        # Test Keyframe default constructor
        kf1 = anim_chop.Keyframe()
        result.assert_equal(0.0, kf1.time, "Keyframe default constructor time")
        result.assert_equal(0.0, kf1.value, "Keyframe default constructor value")
        
        # Test Keyframe constructor with time/value
        kf2 = anim_chop.Keyframe(1.0, 2.0)
        result.assert_equal(1.0, kf2.time, "Keyframe constructor time")
        result.assert_equal(2.0, kf2.value, "Keyframe constructor value")
        
        # Test Keyframe properties access
        result.assert_not_none(kf2.in_handle, "Keyframe.in_handle property")
        result.assert_not_none(kf2.out_handle, "Keyframe.out_handle property")
        result.assert_not_none(kf2.function, "Keyframe.function property")
        result.assert_not_none(kf2.handle_mode, "Keyframe.handle_mode property")
        
        # Test Keyframe property setters
        kf2.time = 5.0
        kf2.value = 6.0
        result.assert_equal(5.0, kf2.time, "Keyframe.time setter")
        result.assert_equal(6.0, kf2.value, "Keyframe.value setter")
        
        # Test handle manipulation
        in_handle = anim_chop.Point(4.0, 5.5)
        out_handle = anim_chop.Point(6.0, 6.5)
        kf2.in_handle = in_handle
        kf2.out_handle = out_handle
        result.assert_equal(4.0, kf2.in_handle.time, "Keyframe.in_handle setter")
        result.assert_equal(6.0, kf2.out_handle.time, "Keyframe.out_handle setter")
        
        # Test function and handle mode setters
        func_enum = anim_chop.Function
        handle_mode_enum = anim_chop.HandleMode
        
        kf2.function = func_enum.LINEAR
        kf2.handle_mode = handle_mode_enum.FREE
        result.assert_equal(func_enum.LINEAR, kf2.function, "Keyframe.function setter")
        result.assert_equal(handle_mode_enum.FREE, kf2.handle_mode, "Keyframe.handle_mode setter")
        
        # Test Keyframe string representation
        kf_str = str(kf2)
        result.assert_true("Keyframe" in kf_str, "Keyframe string representation")
        
        # Test Keyframe equality
        kf3 = anim_chop.Keyframe(5.0, 6.0)
        kf3.function = func_enum.LINEAR
        kf3.handle_mode = handle_mode_enum.FREE
        kf3.in_handle = in_handle
        kf3.out_handle = out_handle
        result.assert_true(kf2 == kf3, "Keyframe equality comparison")
        
    except Exception as e:
        result.record_exception("Keyframe API error", e)


def test_enum_apis(anim_chop, result):
    """Test HandleMode and Function enums"""
    print("\n--- Testing Enum APIs ---")
    
    try:
        # Test Function enum
        func_enum = anim_chop.Function
        result.assert_not_none(func_enum, "Function enum access")
        
        # Test specific Function enum values
        result.assert_not_none(func_enum.CONSTANT, "Function.CONSTANT")
        result.assert_not_none(func_enum.LINEAR, "Function.LINEAR")
        result.assert_not_none(func_enum.BEZIER, "Function.BEZIER")
        
        # Test Function enum value types
        result.assert_true(isinstance(func_enum.CONSTANT, int), "Function.CONSTANT is int")
        result.assert_true(isinstance(func_enum.LINEAR, int), "Function.LINEAR is int")
        result.assert_true(isinstance(func_enum.BEZIER, int), "Function.BEZIER is int")
        
        # Test HandleMode enum
        handle_mode = anim_chop.HandleMode
        result.assert_not_none(handle_mode, "HandleMode enum access")
        
        # # Test specific HandleMode enum values
        result.assert_not_none(handle_mode.FLAT, "HandleMode.FLAT")
        result.assert_not_none(handle_mode.SMOOTH, "HandleMode.SMOOTH")
        result.assert_not_none(handle_mode.ALIGNED, "HandleMode.ALIGNED")
        result.assert_not_none(handle_mode.FREE, "HandleMode.FREE")
        result.assert_not_none(handle_mode.ALIGN_STRICT, "HandleMode.ALIGN_STRICT")
        result.assert_not_none(handle_mode.ALIGN_FLEX, "HandleMode.ALIGN_FLEX")
        result.assert_not_none(handle_mode.ALIGN_ADJUSTABLE, "HandleMode.ALIGN_ADJUSTABLE")
        
        # Test HandleMode enum value types
        result.assert_true(isinstance(handle_mode.FLAT, int), "HandleMode.FLAT is int")
        result.assert_true(isinstance(handle_mode.FREE, int), "HandleMode.FREE is int")
        
    except Exception as e:
        result.record_exception("Enum API error", e)


def test_animation_chop_core_api(anim_chop, result):
    """Test core AnimationCHOP functionality"""
    print("\n--- Testing AnimationCHOP Core API ---")
    
    try:
        # Test clear method
        anim_chop.clear()
        result.assert_true(True, "AnimationCHOP.clear method")
        
        # Test initial state
        result.assert_equal(0, anim_chop.num_channels, "Initial num_channels is 0")
        result.assert_equal([], anim_chop.channel_names, "Initial channel_names is empty")
        result.assert_equal([], anim_chop.channels, "Initial channels is empty")
        
        # Test channel creation methods
        channel1 = anim_chop.create_channel("test_channel_1")
        result.assert_not_none(channel1, "create_channel method")
        result.assert_equal("test_channel_1", channel1.name, "Created channel has correct name")
        
        # Create second channel without specifying index (append at end)
        channel2 = anim_chop.create_channel("test_channel_2")
        result.assert_not_none(channel2, "create_channel without index")
        
        # Now test create_channel with valid index (insert at beginning)cd 
        print(f"Createing channel at index 0, current num_channels: {anim_chop.num_channels}")
        channel3 = anim_chop.create_channel("test_channel_0", 0)
        result.assert_not_none(channel3, "create_channel with index 0")
        
        # Test channel count and access
        result.assert_equal(3, anim_chop.num_channels, "num_channels after creating 3 channels")
        
        channel_names = anim_chop.channel_names
        result.assert_equal(3, len(channel_names), "channel_names length")
        result.assert_true("test_channel_1" in channel_names, "channel_names contains created channel")
        result.assert_true("test_channel_2" in channel_names, "channel_names contains created channel")
        result.assert_true("test_channel_0" in channel_names, "channel_names contains inserted channel")
        
        channels = anim_chop.channels
        result.assert_equal(3, len(channels), "channels list length")
        
        # Test get_channel by name and index
        ch_by_name = anim_chop.get_channel("test_channel_1")
        result.assert_not_none(ch_by_name, "get_channel by name")
        result.assert_equal("test_channel_1", ch_by_name.name, "get_channel by name returns correct channel")
        
        ch_by_index = anim_chop.get_channel(0)
        result.assert_not_none(ch_by_index, "get_channel by index")
        # After insertion at 0, the first channel should be test_channel_0
        result.assert_equal("test_channel_0", ch_by_index.name, "get_channel by index 0 returns inserted channel")
        
        # Test has_channel
        result.assert_true(anim_chop.has_channel("test_channel_1"), "has_channel for existing channel")
        result.assert_false(anim_chop.has_channel("nonexistent"), "has_channel for non-existing channel")
        
        # Test animation timing properties
        result.assert_not_none(anim_chop.start_time, "start_time property")
        result.assert_not_none(anim_chop.end_time, "end_time property")
        result.assert_not_none(anim_chop.length, "length property")
        
        # Test timing setters
        anim_chop.start_time = 1.0
        result.assert_equal(1.0, anim_chop.start_time, "start_time setter")
        
        anim_chop.end_time = 5.0
        result.assert_equal(5.0, anim_chop.end_time, "end_time setter")
        
        anim_chop.length = 10.0
        result.assert_equal(10.0, anim_chop.length, "length setter")
        
        # Test channel removal
        anim_chop.remove_channel("test_channel_1")
        result.assert_equal(2, anim_chop.num_channels, "num_channels after removing channel by name")
        result.assert_false(anim_chop.has_channel("test_channel_1"), "removed channel no longer exists")
        
        anim_chop.remove_channel(0)  # Remove by index
        result.assert_equal(1, anim_chop.num_channels, "num_channels after removing channel by index")
        
    except Exception as e:
        result.record_exception("AnimationCHOP Core API error", e)


def test_channel_api(anim_chop, result):
    """Test Channel class functionality"""
    print("\n--- Testing Channel API ---")
    
    try:
        # Create a test channel
        anim_chop.clear()
        channel = anim_chop.create_channel("test_channel")
        
        # Test initial channel state
        result.assert_equal("test_channel", channel.name, "Channel.name property")
        result.assert_equal(0, channel.size, "Channel.size initial value")
        result.assert_equal(0, channel.num_keyframes, "Channel.num_keyframes initial value")
        result.assert_true(channel.empty, "Channel.empty initial value")
        
        # Test channel name setter
        channel.name = "renamed_channel"
        result.assert_equal("renamed_channel", channel.name, "Channel.name setter")
        
        # Test keyframe creation methods
        kf1 = channel.create_keyframe(0.0, 1.0)
        result.assert_not_none(kf1, "Channel.create_keyframe method")
        result.assert_equal(0.0, kf1.time, "Created keyframe time")
        result.assert_equal(1.0, kf1.value, "Created keyframe value")
        
        # Test channel state after keyframe creation
        result.assert_equal(1, channel.size, "Channel.size after adding keyframe")
        result.assert_equal(1, channel.num_keyframes, "Channel.num_keyframes after adding keyframe")
        result.assert_false(channel.empty, "Channel.empty after adding keyframe")
        
        # Test keyframe creation with Point
        point = anim_chop.Point(2.0, 3.0)
        kf2 = channel.create_keyframe(point)
        result.assert_equal(2.0, kf2.time, "Create keyframe with Point")
        result.assert_equal(3.0, kf2.value, "Create keyframe with Point")
        
        # Test keyframe creation with handles
        in_handle = anim_chop.Point(3.5, 2.0)
        out_handle = anim_chop.Point(4.5, 4.0)
        kf3 = channel.create_keyframe(4.0, 5.0, in_handle, out_handle, 
                                       anim_chop.Function.BEZIER, anim_chop.HandleMode.FREE)
        result.assert_equal(4.0, kf3.time, "Create keyframe with handles")
        result.assert_equal(3.5, kf3.in_handle.time, "Keyframe in_handle set correctly")
        result.assert_equal(4.5, kf3.out_handle.time, "Keyframe out_handle set correctly")
        
        # Test keyframe access by index
        retrieved_kf = channel.keyframe(0)
        result.assert_equal(0.0, retrieved_kf.time, "Channel.keyframe() by index")
        
        # Test sequence protocol (len, getitem, contains)
        result.assert_equal(3, len(channel), "Channel len() protocol")
        
        kf_by_index = channel[1]
        result.assert_equal(2.0, kf_by_index.time, "Channel[index] protocol")
        
        result.assert_true(0.0 in channel, "Channel contains protocol - existing time")
        result.assert_false(10.0 in channel, "Channel contains protocol - non-existing time")
        
        # Test keyframe queries
        prev_kf = channel.prev_keyframe(3.0)
        result.assert_equal(2.0, prev_kf.time, "Channel.prev_keyframe method")
        
        next_kf = channel.next_keyframe(1.0)
        result.assert_equal(2.0, next_kf.time, "Channel.next_keyframe method")
        
        closest_kf = channel.closest_keyframe(1.8)
        result.assert_equal(2.0, closest_kf.time, "Channel.closest_keyframe method")
        
        # Test keyframe modification methods
        new_kf = anim_chop.Keyframe(time=1.0, value=10.0, handle_mode=anim_chop.HandleMode.FREE)
        channel.update_keyframe(1, new_kf)
        updated_kf = channel.keyframe(1)
        result.assert_equal(1.0, updated_kf.time, "Channel.update_keyframe method")
        result.assert_equal(10.0, updated_kf.value, "Channel.update_keyframe method")
        
        channel.set_keyframe_time(1, 1.5)
        result.assert_equal(1.5, channel.keyframe(1).time, "Channel.set_keyframe_time method")
        
        channel.set_keyframe_value(1, 15.0)
        result.assert_equal(15.0, channel.keyframe(1).value, "Channel.set_keyframe_value method")
        
        new_point = anim_chop.Point(1.8, 18.0)
        channel.set_keyframe_position(1, new_point)
        result.assert_equal(1.8, channel.keyframe(1).time, "Channel.set_keyframe_position with Point")
        
        channel.set_keyframe_position(1, 1.9, 19.0)
        result.assert_equal(1.9, channel.keyframe(1).time, "Channel.set_keyframe_position with time,value")
        result.assert_equal(19.0, channel.keyframe(1).value, "Channel.set_keyframe_position with time,value")
        
        # Test handle setters
        new_in_handle = anim_chop.Point(1.0, 18.0)
        new_out_handle = anim_chop.Point(2.8, 20.0)
        channel.set_keyframe_in_handle(1, new_in_handle)
        channel.set_keyframe_out_handle(1, new_out_handle)
        updated_kf = channel.keyframe(1)
        result.assert_equal(1.0, updated_kf.in_handle.time, "Channel.set_keyframe_in_handle")
        result.assert_equal(2.8, updated_kf.out_handle.time, "Channel.set_keyframe_out_handle")
        
        # Test function and handle mode setters
        func_enum = anim_chop.Function
        handle_mode_enum = anim_chop.HandleMode
        
        channel.set_keyframe_function(1, func_enum.LINEAR)
        result.assert_equal(func_enum.LINEAR, channel.keyframe(1).function, "Channel.set_keyframe_function")
        
        channel.set_keyframe_handle_mode(1, handle_mode_enum.FREE)
        result.assert_equal(handle_mode_enum.FREE, channel.keyframe(1).handle_mode, "Channel.set_keyframe_handle_mode")
        
        # Test evaluation
        value_at_0 = channel.evaluate(0.0)
        result.assert_true(isinstance(value_at_0, float), "Channel.evaluate returns float")
        result.assert_equal(1.0, value_at_0, "Channel.evaluate at keyframe time")
        
        # Test evaluation ranges
        values_range = channel.evaluate_range(0.0, 4.0, 5)
        result.assert_equal(5, len(values_range), "Channel.evaluate_range returns correct number of samples")
        result.assert_true(all(isinstance(v, float) for v in values_range), "Channel.evaluate_range returns floats")
        
        values_by_rate = channel.evaluate_range_by_rate(0.0, 2.0, 1.0)
        result.assert_equal(3, len(values_by_rate), "Channel.evaluate_range_by_rate returns correct samples")
        
        # Test channel timing properties
        result.assert_equal(0.0, channel.start_time, "Channel.start_time")
        result.assert_equal(4.0, channel.end_time, "Channel.end_time")
        result.assert_equal(4.0, channel.length, "Channel.length")
        
        # Test num_samples calculation
        num_samples = channel.num_samples(30.0)
        result.assert_true(isinstance(num_samples, int), "Channel.num_samples returns int")
        result.assert_true(num_samples > 0, "Channel.num_samples returns positive value")
        
        # Test keyframe removal
        channel.delete_keyframe(1)
        result.assert_equal(2, channel.num_keyframes, "Channel.delete_keyframe method")
        
        # Test channel string representation
        ch_str = str(channel)
        result.assert_true("Channel" in ch_str, "Channel string representation")
        result.assert_true("renamed_channel" in ch_str, "Channel string contains name")
        
    except Exception as e:
        result.record_exception("Channel API error", e)


def test_advanced_features(anim_chop, result):
    """Test advanced AnimationCHOP features and edge cases"""
    print("\n--- Testing Advanced Features ---")
    
    try:
        anim_chop.clear()
        
        # Test complex animation setup
        pos_x = anim_chop.create_channel("pos_x")
        pos_y = anim_chop.create_channel("pos_y")
        rotation = anim_chop.create_channel("rotation")
        
        # Create animation data
        times = [0.0, 1.0, 2.0, 3.0, 4.0]
        x_values = [0.0, 10.0, 20.0, 15.0, 5.0]
        y_values = [0.0, 5.0, 10.0, 25.0, 30.0]
        rot_values = [0.0, 90.0, 180.0, 270.0, 360.0]
        
        func_enum = anim_chop.Function
        handle_mode_enum = anim_chop.HandleMode
        
        # Add keyframes with different interpolation types
        for i, (t, x, y, r) in enumerate(zip(times, x_values, y_values, rot_values)):
            # Vary the interpolation types
            func_type = [func_enum.CONSTANT, func_enum.LINEAR, func_enum.BEZIER, func_enum.LINEAR, func_enum.BEZIER][i]
            handle_type = [handle_mode_enum.FLAT, handle_mode_enum.SMOOTH, handle_mode_enum.FREE, handle_mode_enum.ALIGNED, handle_mode_enum.SMOOTH][i]
            
            pos_x.create_keyframe(t, x, func_type, handle_type)
            pos_y.create_keyframe(t, y, func_type, handle_type)
            rotation.create_keyframe(t, r, func_type, handle_type)
        
        result.assert_equal(5, pos_x.num_keyframes, "Complex animation - pos_x keyframe count")
        result.assert_equal(5, pos_y.num_keyframes, "Complex animation - pos_y keyframe count")
        result.assert_equal(5, rotation.num_keyframes, "Complex animation - rotation keyframe count")
        
        # Test evaluation at various times
        test_times = [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0, 3.5, 4.0]
        for t in test_times:
            x_val = pos_x.evaluate(t)
            y_val = pos_y.evaluate(t)
            r_val = rotation.evaluate(t)
            
            result.assert_true(isinstance(x_val, float), f"pos_x evaluation at t={t}")
            result.assert_true(isinstance(y_val, float), f"pos_y evaluation at t={t}")
            result.assert_true(isinstance(r_val, float), f"rotation evaluation at t={t}")
        
        # Test interpolation behavior
        # Linear interpolation between keyframes 1 and 2
        pos_x.set_keyframe_function(1, func_enum.LINEAR)
        val_at_1_5 = pos_x.evaluate(1.5)
        expected_linear = 10.0 + (20.0 - 10.0) * 0.5  # Should be 15.0
        result.assert_near(expected_linear, val_at_1_5, 0.01, "Linear interpolation test")
        
        # Test emplace_keyframe with existing keyframe
        new_keyframe = anim_chop.Keyframe(2.5, 22.5)
        pos_x.emplace_keyframe(new_keyframe)
        result.assert_equal(6, pos_x.num_keyframes, "emplace_keyframe adds keyframe")
        
        # Fix: Test channel insertion at valid index - we now have 4 channels total
        current_channel_count = anim_chop.num_channels
        insert_channel = anim_chop.create_channel("scale", current_channel_count)  # Insert at end is always valid
        result.assert_equal("scale", anim_chop.get_channel(current_channel_count).name, "Channel insertion at end")
        
        # Test multiple evaluation ranges
        sample_rates = [30.0, 60.0, 120.0]
        for rate in sample_rates:
            samples = pos_x.num_samples(rate)
            values = pos_x.evaluate_range_by_rate(0.0, 4.0, rate)
            expected_samples = int(4.0 * rate) + 1
            result.assert_near(expected_samples, len(values), 1, f"Sample count at {rate} Hz")
        
        result.assert_true(True, "Complex animation scenario completed")
        
    except Exception as e:
        result.record_exception("Advanced features error", e)


def test_error_handling(anim_chop, result):
    """Test error handling and edge cases"""
    print("\n--- Testing Error Handling ---")
    
    try:
        anim_chop.clear()
        
        # Test invalid channel access
        try:
            invalid_channel = anim_chop.get_channel("nonexistent_channel")
            result.assert_true(False, "get_channel with invalid name should raise exception")
        except:
            result.assert_true(True, "get_channel with invalid name raises exception")
        
        try:
            invalid_channel = anim_chop.get_channel(999)
            result.assert_true(False, "get_channel with invalid index should raise exception")
        except:
            result.assert_true(True, "get_channel with invalid index raises exception")
        
        # Test invalid keyframe access
        channel = anim_chop.create_channel("error_test")
        
        try:
            invalid_keyframe = channel.keyframe(999)
            result.assert_true(False, "keyframe with invalid index should raise exception")
        except:
            result.assert_true(True, "keyframe with invalid index raises exception")
        
        try:
            invalid_prev = channel.prev_keyframe(0.0)
            result.assert_true(False, "prev_keyframe on empty channel should raise exception")
        except:
            result.assert_true(True, "prev_keyframe on empty channel raises exception")
        
        # Test invalid removal
        try:
            anim_chop.remove_channel("nonexistent")
            result.assert_true(False, "remove_channel with invalid name should raise exception")
        except:
            result.assert_true(True, "remove_channel with invalid name raises exception")
        
        try:
            channel.delete_keyframe(999)
            result.assert_true(False, "delete_keyframe with invalid index should raise exception")
        except:
            result.assert_true(True, "delete_keyframe with invalid index raises exception")
        
        # Test boundary values
        channel.create_keyframe(-1000.0, -1000.0)
        channel.create_keyframe(1000.0, 1000.0)
        result.assert_equal(2, channel.num_keyframes, "Extreme boundary values accepted")
        
        # Test zero and negative lengths
        try:
            anim_chop.length = -1.0
            result.assert_true(False, "Negative length should raise exception")
        except:
            result.assert_true(True, "Negative length raises exception")
        
        # Test type errors
        try:
            bad_point = anim_chop.Point("invalid", "types")
            result.assert_true(False, "Point with invalid types should raise exception")
        except:
            result.assert_true(True, "Point with invalid types raises exception")
        
    except Exception as e:
        result.record_exception("Error handling test error", e)


def test_state_apis(anim_chop, result):
    """Test state getter/setter functionality for all types"""
    print("\n--- Testing State APIs ---")
    
    try:
        # Test Point state
        print("Testing Point state...")
        point1 = anim_chop.Point(1.5, 2.5)
        
        # Test Point get_state method
        point_state = point1.get_state()
        result.assert_not_none(point_state, "Point.get_state() returns value")
        result.assert_true(isinstance(point_state, dict), "Point.get_state() returns dict")
        result.assert_equal(1.5, point_state['time'], "Point state contains correct time")
        result.assert_equal(2.5, point_state['value'], "Point state contains correct value")
        
        # Test Point state property getter
        point_state_prop = point1.state
        result.assert_equal(point_state, point_state_prop, "Point.state property equals get_state()")
        
        # Test Point set_state method
        new_point_state = {'time': 3.0, 'value': 4.0}
        point1.set_state(new_point_state)
        result.assert_equal(3.0, point1.time, "Point.set_state() updates time")
        result.assert_equal(4.0, point1.value, "Point.set_state() updates value")
        
        # Test Point state property setter
        point2 = anim_chop.Point(0.0, 0.0)
        point2.state = {'time': 5.0, 'value': 6.0}
        result.assert_equal(5.0, point2.time, "Point.state setter updates time")
        result.assert_equal(6.0, point2.value, "Point.state setter updates value")
        
        # Test Keyframe state
        print("Testing Keyframe state...")
        kf = anim_chop.Keyframe(2.0, 3.0)
        kf.function = anim_chop.Function.BEZIER
        kf.handle_mode = anim_chop.HandleMode.FREE
        kf.in_handle = anim_chop.Point(1.5, 2.5)
        kf.out_handle = anim_chop.Point(2.5, 3.5)
        
        # Test Keyframe get_state method
        kf_state = kf.get_state()
        result.assert_not_none(kf_state, "Keyframe.get_state() returns value")
        result.assert_true(isinstance(kf_state, dict), "Keyframe.get_state() returns dict")
        
        # Verify keyframe state structure
        result.assert_true('position' in kf_state, "Keyframe state has position")
        result.assert_true('in_handle' in kf_state, "Keyframe state has in_handle")
        result.assert_true('out_handle' in kf_state, "Keyframe state has out_handle")
        result.assert_true('function' in kf_state, "Keyframe state has function")
        result.assert_true('handle_mode' in kf_state, "Keyframe state has handle_mode")
        
        # Verify keyframe state values
        result.assert_equal(2.0, kf_state['position']['time'], "Keyframe state position time")
        result.assert_equal(3.0, kf_state['position']['value'], "Keyframe state position value")
        result.assert_equal(1.5, kf_state['in_handle']['time'], "Keyframe state in_handle time")
        result.assert_equal(2.5, kf_state['in_handle']['value'], "Keyframe state in_handle value")
        result.assert_equal(2.5, kf_state['out_handle']['time'], "Keyframe state out_handle time")
        result.assert_equal(3.5, kf_state['out_handle']['value'], "Keyframe state out_handle value")
        result.assert_equal('Function.BEZIER', kf_state['function'], "Keyframe state function")
        result.assert_equal('HandleMode.FREE', kf_state['handle_mode'], "Keyframe state handle_mode")
        
        # Test Keyframe state property getter
        kf_state_prop = kf.state
        result.assert_equal(kf_state, kf_state_prop, "Keyframe.state property equals get_state()")
        
        # Test Keyframe set_state method
        new_kf_state = {
            'position': {'time': 4.0, 'value': 5.0},
            'in_handle': {'time': 3.5, 'value': 4.5},
            'out_handle': {'time': 4.5, 'value': 5.5},
            'function': 'Function.LINEAR',
            'handle_mode': 'HandleMode.SMOOTH'
        }
        kf.set_state(new_kf_state)
        result.assert_equal(4.0, kf.time, "Keyframe.set_state() updates time")
        result.assert_equal(5.0, kf.value, "Keyframe.set_state() updates value")
        result.assert_equal(3.5, kf.in_handle.time, "Keyframe.set_state() updates in_handle time")
        result.assert_equal(4.5, kf.out_handle.time, "Keyframe.set_state() updates out_handle time")
        result.assert_equal(anim_chop.Function.LINEAR, kf.function, "Keyframe.set_state() updates function")
        result.assert_equal(anim_chop.HandleMode.SMOOTH, kf.handle_mode, "Keyframe.set_state() updates handle_mode")
        
        # Test Keyframe state property setter
        kf2 = anim_chop.Keyframe(0.0, 0.0)
        kf2.state = {
            'position': {'time': 6.0, 'value': 7.0},
            'in_handle': {'time': 5.5, 'value': 6.5},
            'out_handle': {'time': 6.5, 'value': 7.5},
            'function': 'Function.CONSTANT',
            'handle_mode': 'HandleMode.FLAT'
        }
        result.assert_equal(6.0, kf2.time, "Keyframe.state setter updates time")
        result.assert_equal(7.0, kf2.value, "Keyframe.state setter updates value")
        result.assert_equal(anim_chop.Function.CONSTANT, kf2.function, "Keyframe.state setter updates function")
        result.assert_equal(anim_chop.HandleMode.FLAT, kf2.handle_mode, "Keyframe.state setter updates handle_mode")
        
        # Test Channel state
        print("Testing Channel state...")
        anim_chop.clear()
        channel = anim_chop.create_channel("test_channel")
        
        # Add some keyframes to the channel
        kf1 = channel.create_keyframe(0.0, 1.0, anim_chop.Function.LINEAR, anim_chop.HandleMode.SMOOTH)
        kf2 = channel.create_keyframe(1.0, 2.0, anim_chop.Function.BEZIER, anim_chop.HandleMode.FREE)
        kf3 = channel.create_keyframe(2.0, 1.5, anim_chop.Function.CONSTANT, anim_chop.HandleMode.FLAT)
        
        # Test Channel get_state method
        ch_state = channel.get_state()
        result.assert_not_none(ch_state, "Channel.get_state() returns value")
        result.assert_true(isinstance(ch_state, dict), "Channel.get_state() returns dict")
        
        # Verify channel state structure
        result.assert_true('name' in ch_state, "Channel state has name")
        result.assert_true('start_time' in ch_state, "Channel state has start_time")
        result.assert_true('end_time' in ch_state, "Channel state has end_time")
        result.assert_true('length' in ch_state, "Channel state has length")
        result.assert_true('num_keyframes' in ch_state, "Channel state has num_keyframes")
        result.assert_true('empty' in ch_state, "Channel state has empty")
        result.assert_true('keyframes' in ch_state, "Channel state has keyframes")
        
        # Verify channel state values
        result.assert_equal("test_channel", ch_state['name'], "Channel state name")
        result.assert_equal(0.0, ch_state['start_time'], "Channel state start_time")
        result.assert_equal(2.0, ch_state['end_time'], "Channel state end_time")
        result.assert_equal(2.0, ch_state['length'], "Channel state length")
        result.assert_equal(3, ch_state['num_keyframes'], "Channel state num_keyframes")
        result.assert_equal(False, ch_state['empty'], "Channel state empty")
        result.assert_equal(3, len(ch_state['keyframes']), "Channel state keyframes length")
        
        # Verify keyframes in channel state are dictionaries
        for i, kf_state in enumerate(ch_state['keyframes']):
            result.assert_true(isinstance(kf_state, dict), f"Channel state keyframe {i} is dict")
            result.assert_true('position' in kf_state, f"Channel state keyframe {i} has position")
            result.assert_true('function' in kf_state, f"Channel state keyframe {i} has function")
            result.assert_true('handle_mode' in kf_state, f"Channel state keyframe {i} has handle_mode")
        
        # Test Channel state property getter
        ch_state_prop = channel.state
        result.assert_equal(ch_state, ch_state_prop, "Channel.state property equals get_state()")
        
        # Test Channel set_state method with a new channel
        new_ch_state = {
            'name': 'restored_channel',
            'keyframes': [
                {
                    'position': {'time': 0.5, 'value': 10.0},
                    'in_handle': {'time': 0.2, 'value': 9.5},
                    'out_handle': {'time': 0.8, 'value': 10.5},
                    'function': anim_chop.Function.BEZIER,
                    'handle_mode': anim_chop.HandleMode.FREE
                },
                {
                    'position': {'time': 1.5, 'value': 15.0},
                    'in_handle': {'time': 1.2, 'value': 14.5},
                    'out_handle': {'time': 1.8, 'value': 15.5},
                    'function': anim_chop.Function.LINEAR,
                    'handle_mode': anim_chop.HandleMode.SMOOTH
                }
            ]
        }
        
        # Clear and set new state
        channel.set_state(new_ch_state)
        result.assert_equal("restored_channel", channel.name, "Channel.set_state() updates name")
        result.assert_equal(2, channel.num_keyframes, "Channel.set_state() restores keyframes")
        result.assert_equal(0.5, channel.keyframe(0).time, "Channel.set_state() restores keyframe time")
        result.assert_equal(10.0, channel.keyframe(0).value, "Channel.set_state() restores keyframe value")
        result.assert_equal(anim_chop.Function.BEZIER, channel.keyframe(0).function, "Channel.set_state() restores keyframe function")
        
        # Test Channel state property setter
        channel2 = anim_chop.create_channel("temp_channel")
        channel2.state = {
            'name': 'state_setter_channel',
            'keyframes': [
                {
                    'position': {'time': 3.0, 'value': 30.0},
                    'function': anim_chop.Function.CONSTANT,
                    'handle_mode': anim_chop.HandleMode.FLAT
                }
            ]
        }
        result.assert_equal("state_setter_channel", channel2.name, "Channel.state setter updates name")
        result.assert_equal(1, channel2.num_keyframes, "Channel.state setter restores keyframes")
        result.assert_equal(3.0, channel2.keyframe(0).time, "Channel.state setter restores keyframe time")
        
        # Test AnimationCHOP state
        print("Testing AnimationCHOP state...")
        anim_chop.clear()
        
        # Create a complex animation for state testing
        ch1 = anim_chop.create_channel("pos_x")
        ch1.create_keyframe(0.0, 0.0, anim_chop.Function.LINEAR, anim_chop.HandleMode.SMOOTH)
        ch1.create_keyframe(2.0, 10.0, anim_chop.Function.BEZIER, anim_chop.HandleMode.FREE)
        
        ch2 = anim_chop.create_channel("pos_y")
        ch2.create_keyframe(0.5, 5.0, anim_chop.Function.CONSTANT, anim_chop.HandleMode.FLAT)
        ch2.create_keyframe(1.5, 15.0, anim_chop.Function.LINEAR, anim_chop.HandleMode.SMOOTH)
        
        anim_chop.start_time = 0.0
        anim_chop.end_time = 3.0
        
        # Test AnimationCHOP get_state method
        anim_state = anim_chop.get_state()
        result.assert_not_none(anim_state, "AnimationCHOP.get_state() returns value")
        result.assert_true(isinstance(anim_state, dict), "AnimationCHOP.get_state() returns dict")
        
        # Verify animation state structure
        result.assert_true('start_time' in anim_state, "Animation state has start_time")
        result.assert_true('end_time' in anim_state, "Animation state has end_time")
        result.assert_true('length' in anim_state, "Animation state has length")
        result.assert_true('num_channels' in anim_state, "Animation state has num_channels")
        result.assert_true('channels' in anim_state, "Animation state has channels")
        
        # Verify animation state values
        result.assert_equal(0.0, anim_state['start_time'], "Animation state start_time")
        result.assert_equal(3.0, anim_state['end_time'], "Animation state end_time")
        result.assert_equal(3.0, anim_state['length'], "Animation state length")
        result.assert_equal(2, anim_state['num_channels'], "Animation state num_channels")
        result.assert_equal(2, len(anim_state['channels']), "Animation state channels length")
        
        # Verify channels in animation state
        ch1_state = anim_state['channels'][0]
        ch2_state = anim_state['channels'][1]
        result.assert_equal("pos_x", ch1_state['name'], "Animation state channel 1 name")
        result.assert_equal("pos_y", ch2_state['name'], "Animation state channel 2 name")
        result.assert_equal(2, len(ch1_state['keyframes']), "Animation state channel 1 keyframes")
        result.assert_equal(2, len(ch2_state['keyframes']), "Animation state channel 2 keyframes")
        
        # Test AnimationCHOP state property getter
        anim_state_prop = anim_chop.state
        result.assert_equal(anim_state, anim_state_prop, "AnimationCHOP.state property equals get_state()")
        
        # Test AnimationCHOP set_state method
        new_anim_state = {
            'start_time': 1.0,
            'end_time': 5.0,
            'channels': [
                {
                    'name': 'restored_x',
                    'keyframes': [
                        {
                            'position': {'time': 1.0, 'value': 100.0},
                            'function': anim_chop.Function.LINEAR,
                            'handle_mode': anim_chop.HandleMode.SMOOTH
                        },
                        {
                            'position': {'time': 3.0, 'value': 200.0},
                            'function': anim_chop.Function.BEZIER,
                            'handle_mode': anim_chop.HandleMode.FREE
                        }
                    ]
                },
                {
                    'name': 'restored_y',
                    'keyframes': [
                        {
                            'position': {'time': 2.0, 'value': 150.0},
                            'function': anim_chop.Function.CONSTANT,
                            'handle_mode': anim_chop.HandleMode.FLAT
                        }
                    ]
                }
            ]
        }
        
        anim_chop.set_state(new_anim_state)
        result.assert_equal(1.0, anim_chop.start_time, "AnimationCHOP.set_state() updates start_time")
        result.assert_equal(5.0, anim_chop.end_time, "AnimationCHOP.set_state() updates end_time")
        result.assert_equal(2, anim_chop.num_channels, "AnimationCHOP.set_state() restores channels")
        result.assert_equal("restored_x", anim_chop.get_channel(0).name, "AnimationCHOP.set_state() restores channel names")
        result.assert_equal("restored_y", anim_chop.get_channel(1).name, "AnimationCHOP.set_state() restores channel names")
        result.assert_equal(2, anim_chop.get_channel(0).num_keyframes, "AnimationCHOP.set_state() restores keyframes")
        result.assert_equal(1, anim_chop.get_channel(1).num_keyframes, "AnimationCHOP.set_state() restores keyframes")
        
        # Test AnimationCHOP state property setter
        anim_chop.state = {
            'start_time': 0.0,
            'end_time': 2.0,
            'channels': [
                {
                    'name': 'final_test',
                    'keyframes': [
                        {
                            'position': {'time': 0.0, 'value': 50.0},
                            'function': anim_chop.Function.LINEAR,
                            'handle_mode': anim_chop.HandleMode.SMOOTH
                        }
                    ]
                }
            ]
        }
        result.assert_equal(0.0, anim_chop.start_time, "AnimationCHOP.state setter updates start_time")
        result.assert_equal(2.0, anim_chop.end_time, "AnimationCHOP.state setter updates end_time")
        result.assert_equal(1, anim_chop.num_channels, "AnimationCHOP.state setter restores channels")
        result.assert_equal("final_test", anim_chop.get_channel(0).name, "AnimationCHOP.state setter restores channel name")
        
    except Exception as e:
        result.record_exception("State APIs error", e)


def test_state_error_handling(anim_chop, result):
    """Test state error handling and edge cases"""
    print("\n--- Testing State Error Handling ---")
    
    try:
        # Test invalid Point state
        point = anim_chop.Point(1.0, 2.0)
        
        # Test invalid state types
        try:
            point.set_state("invalid")
            result.assert_true(False, "Point.set_state() with string should raise exception")
        except:
            result.assert_true(True, "Point.set_state() with invalid type raises exception")
        
        try:
            point.state = ["invalid", "list"]
            result.assert_true(False, "Point.state setter with list should raise exception")
        except:
            result.assert_true(True, "Point.state setter with invalid type raises exception")
        
        # Test missing required fields
        try:
            point.set_state({'time': 1.0})  # Missing value
            result.assert_true(False, "Point.set_state() with missing value should raise exception")
        except:
            result.assert_true(True, "Point.set_state() with missing field raises exception")
        
        try:
            point.set_state({'value': 2.0})  # Missing time
            result.assert_true(False, "Point.set_state() with missing time should raise exception")
        except:
            result.assert_true(True, "Point.set_state() with missing field raises exception")
        
        # Test invalid field types in Point state
        try:
            point.set_state({'time': "invalid", 'value': 2.0})
            result.assert_true(False, "Point.set_state() with invalid time type should raise exception")
        except:
            result.assert_true(True, "Point.set_state() with invalid field type raises exception")
        
        # Test invalid Keyframe state
        kf = anim_chop.Keyframe(1.0, 2.0)
        
        try:
            kf.set_state(123)  # Not a dict
            result.assert_true(False, "Keyframe.set_state() with number should raise exception")
        except:
            result.assert_true(True, "Keyframe.set_state() with invalid type raises exception")
        
        try:
            kf.set_state({'position': 'invalid'})  # Position not a dict
            result.assert_true(False, "Keyframe.set_state() with invalid position should raise exception")
        except:
            result.assert_true(True, "Keyframe.set_state() with invalid position raises exception")
        
        try:
            kf.set_state({'position': {'time': 1.0}})  # Missing value in position
            result.assert_true(False, "Keyframe.set_state() with incomplete position should raise exception")
        except:
            result.assert_true(True, "Keyframe.set_state() with incomplete position raises exception")
        
        # Test invalid Channel state
        anim_chop.clear()
        channel = anim_chop.create_channel("test_channel")
        
        try:
            channel.set_state(None)
            result.assert_true(False, "Channel.set_state() with None should raise exception")
        except:
            result.assert_true(True, "Channel.set_state() with None raises exception")
        
        try:
            channel.set_state({'keyframes': 'invalid'})  # Keyframes not a list
            result.assert_true(False, "Channel.set_state() with invalid keyframes should raise exception")
        except:
            result.assert_true(True, "Channel.set_state() with invalid keyframes raises exception")
        
        # Test invalid AnimationCHOP state
        try:
            anim_chop.set_state({'channels': 'invalid'})  # Channels not a list
            result.assert_true(False, "AnimationCHOP.set_state() with invalid channels should raise exception")
        except:
            result.assert_true(True, "AnimationCHOP.set_state() with invalid channels raises exception")
        
        try:
            anim_chop.set_state({'channels': []})  # Missing name in channel
            result.assert_true(True, "AnimationCHOP.set_state() with empty channels succeeds")
        except Exception as e:
            result.record_exception("Unexpected error with empty channels", e)
        
        try:
            anim_chop.set_state({'channels': [{'keyframes': []}]})  # Missing name in channel
            result.assert_true(False, "AnimationCHOP.set_state() with nameless channel should raise exception")
        except:
            result.assert_true(True, "AnimationCHOP.set_state() with nameless channel raises exception")
        
    except Exception as e:
        result.record_exception("State error handling test error", e)


def test_state_roundtrip(anim_chop, result):
    """Test state roundtrip - save and restore should be identical"""
    print("\n--- Testing State Roundtrip ---")
    
    try:
        anim_chop.clear()
        
        # Create a complex animation
        ch1 = anim_chop.create_channel("roundtrip_x")
        ch1.create_keyframe(0.0, 10.0, anim_chop.Function.BEZIER, anim_chop.HandleMode.FREE)
        ch1.create_keyframe(1.5, 25.0, anim_chop.Function.LINEAR, anim_chop.HandleMode.SMOOTH)
        ch1.create_keyframe(3.0, 5.0, anim_chop.Function.CONSTANT, anim_chop.HandleMode.FLAT)
        
        ch2 = anim_chop.create_channel("roundtrip_y")
        ch2.create_keyframe(0.5, 15.0, anim_chop.Function.BEZIER, anim_chop.HandleMode.ALIGNED)
        ch2.create_keyframe(2.5, 35.0, anim_chop.Function.LINEAR, anim_chop.HandleMode.FREE)
        
        # Set specific handle positions for bezier keyframes
        kf0 = ch1.keyframe(0)
        kf0.in_handle = anim_chop.Point(-0.3, 8.0)
        kf0.out_handle = anim_chop.Point(0.3, 12.0)
        ch1.update_keyframe(0, kf0)
        
        kf0_ch2 = ch2.keyframe(0)
        kf0_ch2.in_handle = anim_chop.Point(0.2, 13.0)
        kf0_ch2.out_handle = anim_chop.Point(0.8, 17.0)
        ch2.update_keyframe(0, kf0_ch2)
        
        anim_chop.start_time = 0.0
        anim_chop.end_time = 4.0
        
        # Save original state
        original_state = anim_chop.get_state()
        
        # Save original values for comparison
        orig_ch1_kf0_time = ch1.keyframe(0).time
        orig_ch1_kf0_value = ch1.keyframe(0).value
        orig_ch1_kf0_function = ch1.keyframe(0).function
        orig_ch1_kf0_handle_mode = ch1.keyframe(0).handle_mode
        orig_ch1_kf0_in_handle_time = ch1.keyframe(0).in_handle.time
        orig_ch1_kf0_out_handle_time = ch1.keyframe(0).out_handle.time
        
        orig_ch2_kf0_time = ch2.keyframe(0).time
        orig_ch2_kf0_value = ch2.keyframe(0).value
        
        orig_start_time = anim_chop.start_time
        orig_end_time = anim_chop.end_time
        orig_num_channels = anim_chop.num_channels
        orig_channel_names = anim_chop.channel_names[:]
        
        # Clear and restore from state
        anim_chop.clear()
        result.assert_equal(0, anim_chop.num_channels, "Animation cleared before restore")
        
        anim_chop.set_state(original_state)
        
        # Verify restoration
        result.assert_equal(orig_start_time, anim_chop.start_time, "Roundtrip: start_time restored")
        result.assert_equal(orig_end_time, anim_chop.end_time, "Roundtrip: end_time restored")
        result.assert_equal(orig_num_channels, anim_chop.num_channels, "Roundtrip: num_channels restored")
        result.assert_equal(orig_channel_names, anim_chop.channel_names, "Roundtrip: channel_names restored")
        
        # Verify channel 1 restoration
        restored_ch1 = anim_chop.get_channel("roundtrip_x")
        result.assert_not_none(restored_ch1, "Roundtrip: channel 1 exists")
        result.assert_equal(3, restored_ch1.num_keyframes, "Roundtrip: channel 1 keyframe count")
        
        restored_kf0 = restored_ch1.keyframe(0)
        result.assert_equal(orig_ch1_kf0_time, restored_kf0.time, "Roundtrip: keyframe time")
        result.assert_equal(orig_ch1_kf0_value, restored_kf0.value, "Roundtrip: keyframe value")
        result.assert_equal(orig_ch1_kf0_function, restored_kf0.function, "Roundtrip: keyframe function")
        result.assert_equal(orig_ch1_kf0_handle_mode, restored_kf0.handle_mode, "Roundtrip: keyframe handle_mode")
        result.assert_near(orig_ch1_kf0_in_handle_time, restored_kf0.in_handle.time, 0.001, "Roundtrip: keyframe in_handle time")
        result.assert_near(orig_ch1_kf0_out_handle_time, restored_kf0.out_handle.time, 0.001, "Roundtrip: keyframe out_handle time")
        
        # Verify channel 2 restoration
        restored_ch2 = anim_chop.get_channel("roundtrip_y")
        result.assert_not_none(restored_ch2, "Roundtrip: channel 2 exists")
        result.assert_equal(2, restored_ch2.num_keyframes, "Roundtrip: channel 2 keyframe count")
        
        restored_kf0_ch2 = restored_ch2.keyframe(0)
        result.assert_equal(orig_ch2_kf0_time, restored_kf0_ch2.time, "Roundtrip: channel 2 keyframe time")
        result.assert_equal(orig_ch2_kf0_value, restored_kf0_ch2.value, "Roundtrip: channel 2 keyframe value")
        
        # Verify state is identical after roundtrip
        restored_state = anim_chop.get_state()
        result.assert_equal(original_state, restored_state, "Roundtrip: state is identical after restore")
        
        # Test evaluation consistency
        test_times = [0.0, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0]
        for t in test_times:
            try:
                val_ch1 = restored_ch1.evaluate(t)
                val_ch2 = restored_ch2.evaluate(t)
                result.assert_true(isinstance(val_ch1, float), f"Roundtrip: evaluation at t={t} returns float")
                result.assert_true(isinstance(val_ch2, float), f"Roundtrip: evaluation at t={t} returns float")
            except Exception as e:
                result.record_exception(f"Roundtrip evaluation error at t={t}", e)
        
    except Exception as e:
        result.record_exception("State roundtrip test error", e)


def run_tests(cleanup=False):
    # Get the current operator (this should be called from the AnimationCHOP node)
    try:
        # In TouchDesigner, 'me' refers to the current operator
        anim_chop = op('Animation1')
        print(f"Running tests on node: {anim_chop}")
    except NameError:
        print("ERROR: This script must be run from within TouchDesigner")
        print("Use: op('your_animationchop_name').run_tests()")
        return
    
    try:
        anim_chop.clear()  # Clear any existing channels
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
        test_point_api(anim_chop, result)
        test_keyframe_api(anim_chop, result)
        test_enum_apis(anim_chop, result)
        test_animation_chop_core_api(anim_chop, result)
        test_channel_api(anim_chop, result)
        test_advanced_features(anim_chop, result)
        test_error_handling(anim_chop, result)
        test_state_apis(anim_chop, result)
        test_state_error_handling(anim_chop, result)
        test_state_roundtrip(anim_chop, result)

    except Exception as e:
        print(f"\nUNEXPECTED ERROR: {e}")
        print(traceback.format_exc())
        result.failed += 1
        result.errors.append(f"Unexpected error: {e}")
    
    finally:
        if cleanup:
            anim_chop.clear()
        else:
            print("\nSkipping cleanup. Test channels will remain in AnimationCHOP.")

    # Print final results
    result.print_summary()
    
    return result

run_tests()
