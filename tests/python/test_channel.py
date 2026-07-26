"""Channel: keyframe management, evaluation, extend behaviour and state."""

import pytest


# --- identity and size ------------------------------------------------------

def test_new_channel_is_named_and_empty(channel):
    assert channel.name == "tx"
    assert channel.empty is True
    assert channel.num_keyframes == 0


def test_size_tracks_num_keyframes(channel):
    channel.create_keyframe(0, 0)
    channel.create_keyframe(10, 1)
    assert channel.num_keyframes == 2
    assert channel.size == channel.num_keyframes
    assert channel.empty is False


# --- create_keyframe overloads ---------------------------------------------

def test_create_keyframe_time_value(channel):
    kf = channel.create_keyframe(12, 34)
    assert kf.time == pytest.approx(12.0)
    assert kf.value == pytest.approx(34.0)


def test_create_keyframe_with_function_and_handle_mode(op, channel):
    kf = channel.create_keyframe(0, 0, op.Function.LINEAR, op.HandleMode.FLAT)
    assert kf.function == op.Function.LINEAR
    assert kf.handle_mode == op.HandleMode.FLAT


def test_create_keyframe_from_state(op, channel):
    channel.create_keyframe(0, 0)
    source = channel.keyframe(0).state

    other = op.create_channel("copy")
    made = other.create_keyframe_from_state(source)
    assert made.time == pytest.approx(0.0)
    assert made.state["function"] == source["function"]


def test_keyframes_stay_sorted_by_time(channel):
    for time in (30, 0, 15):
        channel.create_keyframe(time, time)
    times = [channel.keyframe(i).time for i in range(channel.num_keyframes)]
    assert times == sorted(times)


# --- bounds -----------------------------------------------------------------

def test_start_end_and_length(ramp):
    assert ramp.start_time == pytest.approx(0.0)
    assert ramp.end_time == pytest.approx(30.0)
    assert ramp.length == pytest.approx(30.0)


# --- evaluation -------------------------------------------------------------

def test_evaluate_hits_the_keyframes(ramp):
    assert ramp.evaluate(0) == pytest.approx(0.0)
    assert ramp.evaluate(30) == pytest.approx(100.0)


def test_evaluate_midpoint_is_between(ramp):
    assert 0.0 < ramp.evaluate(15) < 100.0


def test_evaluate_at_last_keyframe_is_exact(ramp):
    """Regression: the end-of-range lookup used to dereference end()."""
    assert ramp.evaluate(30.0) == pytest.approx(100.0)


def test_evaluate_range_length(ramp):
    samples = ramp.evaluate_range(0, 30, 31)
    assert len(samples) == 31
    assert samples[0] == pytest.approx(0.0)
    assert samples[-1] == pytest.approx(100.0)


def test_evaluate_range_by_rate(ramp):
    samples = ramp.evaluate_range_by_rate(0, 30, 1.0)
    assert len(samples) > 1
    assert samples[0] == pytest.approx(0.0)


# --- navigation -------------------------------------------------------------

def test_next_and_prev_keyframe(ramp):
    assert ramp.next_keyframe(10).time == pytest.approx(30.0)
    assert ramp.prev_keyframe(10).time == pytest.approx(0.0)


def test_closest_keyframe(ramp):
    assert ramp.closest_keyframe(2).time == pytest.approx(0.0)
    assert ramp.closest_keyframe(28).time == pytest.approx(30.0)


# --- mutation ---------------------------------------------------------------

def test_delete_keyframe(ramp):
    ramp.delete_keyframe(0)
    assert ramp.num_keyframes == 1
    assert ramp.keyframe(0).time == pytest.approx(30.0)


def test_set_keyframe_value(ramp):
    ramp.set_keyframe_value(1, 250.0)
    assert ramp.keyframe(1).value == pytest.approx(250.0)


def test_set_keyframe_time(ramp):
    ramp.set_keyframe_time(1, 60.0)
    assert ramp.keyframe(1).time == pytest.approx(60.0)
    assert ramp.end_time == pytest.approx(60.0)


def test_set_keyframe_position(ramp):
    ramp.set_keyframe_position(1, 45.0, 75.0)
    assert ramp.keyframe(1).time == pytest.approx(45.0)
    assert ramp.keyframe(1).value == pytest.approx(75.0)


# --- sequence protocol ------------------------------------------------------

def test_len_is_the_keyframe_count(ramp):
    assert len(ramp) == 2


def test_subscript_reads_a_keyframe(ramp):
    assert ramp[0].time == pytest.approx(0.0)
    assert ramp[1].value == pytest.approx(100.0)


def test_subscript_accepts_negative_indices(ramp):
    assert ramp[-1].time == pytest.approx(30.0)
    assert ramp[-2].time == pytest.approx(0.0)


def test_iteration_yields_keyframes_in_time_order(ramp):
    assert [kf.time for kf in ramp] == pytest.approx([0.0, 30.0])


def test_subscript_out_of_range_raises_index_error(ramp):
    with pytest.raises(IndexError):
        ramp[99]


def test_subscript_assignment_writes_the_keyframe_back(ramp):
    kf = ramp[0]
    kf.value = 42.0
    ramp[0] = kf

    assert ramp[0].value == pytest.approx(42.0)


def test_subscript_assignment_matches_update_keyframe(op, ramp):
    other = op.create_channel("other")
    other.create_keyframe(0, 0)

    kf = ramp[0]
    kf.value = 42.0
    kf.function = op.Function.LINEAR

    ramp[0] = kf
    other.update_keyframe(0, kf)

    assert ramp[0].value == pytest.approx(other[0].value)
    assert ramp[0].function == other[0].function


def test_subscript_assignment_accepts_negative_indices(ramp):
    kf = ramp[-1]
    kf.value = 7.0
    ramp[-1] = kf

    assert ramp[1].value == pytest.approx(7.0)


def test_subscript_assignment_clamps_time_to_the_neighbour(ramp):
    """Keyframes never reorder: a time past a neighbour clamps to it.

    Writing frame 60 into keyframe 0 pins it at its neighbour's frame 30
    rather than swapping the two, so indices stay stable under time edits.
    """
    kf = ramp[0]
    kf.time = 60.0
    ramp[0] = kf

    assert [k.time for k in ramp] == pytest.approx([30.0, 30.0])


def test_set_keyframe_time_clamps_the_same_way(ramp):
    """The in-place mutator clamps identically -- it is the same code path."""
    ramp.set_keyframe_time(0, 60.0)
    assert [k.time for k in ramp] == pytest.approx([30.0, 30.0])


def test_subscript_assignment_out_of_range_raises_index_error(ramp):
    kf = ramp[0]
    with pytest.raises(IndexError):
        ramp[99] = kf


def test_subscript_assignment_rejects_non_keyframes(ramp):
    with pytest.raises(TypeError):
        ramp[0] = 5.0


def test_subscript_deletion_is_not_supported(ramp):
    """del points at delete_keyframe rather than silently doing nothing."""
    with pytest.raises(TypeError):
        del ramp[0]
    assert ramp.num_keyframes == 2


# --- extend -----------------------------------------------------------------

def test_extend_defaults_to_hold(op, ramp):
    assert ramp.extend_start == op.Extend.HOLD
    assert ramp.extend_end == op.Extend.HOLD


def test_extend_hold_clamps_outside_the_range(ramp):
    assert ramp.evaluate(-50) == pytest.approx(0.0)
    assert ramp.evaluate(500) == pytest.approx(100.0)


def test_extend_is_settable(op, ramp):
    ramp.extend_end = op.Extend.REPEAT
    assert ramp.extend_end == op.Extend.REPEAT


def test_extend_repeat_wraps(op, ramp):
    ramp.extend_end = op.Extend.REPEAT
    # One full period past the end lands back at the start of the cycle.
    assert ramp.evaluate(45) == pytest.approx(ramp.evaluate(15), abs=1e-6)


# --- state ------------------------------------------------------------------

def test_state_shape(ramp):
    state = ramp.state
    assert set(state) == {
        "name", "start_time", "end_time", "length", "num_keyframes",
        "empty", "extend_start", "extend_end", "keyframes",
    }
    assert len(state["keyframes"]) == 2


def test_state_round_trips_through_a_second_channel(op, ramp):
    other = op.create_channel("other")
    other.state = ramp.state

    assert other.num_keyframes == ramp.num_keyframes
    assert other.evaluate(15) == pytest.approx(ramp.evaluate(15))


def test_set_state_replaces_rather_than_appends(op, ramp):
    other = op.create_channel("other")
    other.create_keyframe(0, 0)
    other.create_keyframe(5, 5)
    other.create_keyframe(9, 9)

    other.set_state(ramp.state)
    assert other.num_keyframes == 2


def test_state_preserves_extend(op, ramp):
    ramp.extend_start = op.Extend.MIRROR
    other = op.create_channel("other")
    other.state = ramp.state
    assert other.extend_start == op.Extend.MIRROR


def test_get_state_matches_property(ramp):
    assert ramp.get_state() == ramp.state
