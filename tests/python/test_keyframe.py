"""Keyframe: position, handles, interpolation function and handle mode."""

import pytest


def test_time_and_value(ramp):
    first, last = ramp.keyframe(0), ramp.keyframe(1)
    assert first.time == pytest.approx(0.0)
    assert first.value == pytest.approx(0.0)
    assert last.time == pytest.approx(30.0)
    assert last.value == pytest.approx(100.0)


def test_keyframe_is_a_detached_copy(ramp):
    """keyframe(i) hands back a fresh object each call, not a live view."""
    assert ramp.keyframe(1) is not ramp.keyframe(1)


def test_assigning_to_a_keyframe_does_not_reach_the_channel(ramp):
    """Setting a property on a returned Keyframe updates only that copy.

    The assignment succeeds and reads back on the copy, but the channel is
    untouched -- Channel.set_keyframe_* are the mutators that persist. This
    pins current behaviour; the silent divergence is a sharp edge worth
    revisiting.
    """
    kf = ramp.keyframe(1)
    kf.value = 55.0

    assert kf.value == pytest.approx(55.0)
    assert ramp.keyframe(1).value == pytest.approx(100.0)


def test_set_keyframe_value_persists(ramp):
    ramp.set_keyframe_value(1, 55.0)
    assert ramp.keyframe(1).value == pytest.approx(55.0)


def test_handles_are_points(ramp):
    kf = ramp.keyframe(0)
    assert hasattr(kf.in_handle, "time")
    assert hasattr(kf.out_handle, "value")


def test_defaults_are_bezier_and_smooth(op, ramp):
    kf = ramp.keyframe(0)
    assert kf.function == op.Function.BEZIER
    assert kf.handle_mode == op.HandleMode.SMOOTH


def test_function_is_settable(op, ramp):
    ramp.set_keyframe_function(0, op.Function.LINEAR)
    assert ramp.keyframe(0).function == op.Function.LINEAR


def test_handle_mode_is_settable(op, ramp):
    ramp.set_keyframe_handle_mode(0, op.HandleMode.FLAT)
    assert ramp.keyframe(0).handle_mode == op.HandleMode.FLAT


def test_state_shape(ramp):
    state = ramp.keyframe(0).state
    assert set(state) == {
        "position", "in_handle", "out_handle", "function", "handle_mode",
    }
    assert set(state["position"]) == {"time", "value"}


def test_state_encodes_enums_as_names(ramp):
    state = ramp.keyframe(0).state
    assert state["function"] == "Function.BEZIER"
    assert state["handle_mode"] == "HandleMode.SMOOTH"


def test_state_round_trips(op, ramp):
    kf = ramp.keyframe(0)
    original = kf.state

    kf.function = op.Function.CONSTANT
    assert kf.state["function"] == "Function.CONSTANT"

    kf.state = original
    assert kf.state == original


def test_get_state_matches_property(ramp):
    kf = ramp.keyframe(0)
    assert kf.get_state() == kf.state


def test_linear_function_interpolates_linearly(op, ramp):
    ramp.set_keyframe_function(0, op.Function.LINEAR)
    assert ramp.evaluate(15) == pytest.approx(50.0, abs=1e-6)


def test_constant_function_holds_the_start_value(op, ramp):
    ramp.set_keyframe_function(0, op.Function.CONSTANT)
    assert ramp.evaluate(15) == pytest.approx(0.0)
    assert ramp.evaluate(29.9) == pytest.approx(0.0)
