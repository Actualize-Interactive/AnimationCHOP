"""Point: the time/value pair used for keyframe positions and handles."""

import pytest


def test_handles_expose_time_and_value(ramp):
    handle = ramp.keyframe(0).out_handle
    assert isinstance(handle.time, float)
    assert isinstance(handle.value, float)


def test_state_is_time_and_value(ramp):
    state = ramp.keyframe(0).out_handle.state
    assert set(state) == {"time", "value"}
    assert state["time"] == pytest.approx(ramp.keyframe(0).out_handle.time)
    assert state["value"] == pytest.approx(ramp.keyframe(0).out_handle.value)


def test_get_state_matches_property(ramp):
    handle = ramp.keyframe(0).in_handle
    assert handle.get_state() == handle.state


def test_set_state_round_trips(ramp):
    handle = ramp.keyframe(0).out_handle
    original = handle.state

    handle.set_state({"time": 5.0, "value": 42.0})
    assert handle.time == pytest.approx(5.0)
    assert handle.value == pytest.approx(42.0)

    handle.set_state(original)
    assert handle.state == original


def test_state_setter_property(ramp):
    handle = ramp.keyframe(0).out_handle
    handle.state = {"time": 3.5, "value": -7.25}
    assert handle.state["time"] == pytest.approx(3.5)
    assert handle.state["value"] == pytest.approx(-7.25)


def test_repr_names_the_type(ramp):
    assert "Point" in repr(ramp.keyframe(0).out_handle)
