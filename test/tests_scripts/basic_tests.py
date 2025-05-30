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
        
        # Test specific HandleMode enum values
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
