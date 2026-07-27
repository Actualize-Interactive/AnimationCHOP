"""Error handling: bad input should raise, never crash the host."""

import pytest


def test_get_missing_channel_raises(op):
    with pytest.raises(Exception):
        op.get_channel("nope")


def test_get_channel_out_of_range_raises(op):
    op.create_channel("tx")
    with pytest.raises(Exception):
        op.get_channel(99)


def test_remove_missing_channel_raises(op):
    with pytest.raises(Exception):
        op.remove_channel("nope")


def test_keyframe_index_out_of_range_raises(ramp):
    with pytest.raises(Exception):
        ramp.keyframe(99)


def test_delete_keyframe_out_of_range_raises(ramp):
    with pytest.raises(Exception):
        ramp.delete_keyframe(99)


def test_set_state_rejects_non_mapping(ramp):
    with pytest.raises(TypeError):
        ramp.state = ["not", "a", "dict"]


def test_animation_set_state_rejects_non_mapping(op):
    with pytest.raises(TypeError):
        op.state = 42


def test_animation_set_state_requires_channels(op):
    with pytest.raises(ValueError):
        op.state = {"start_time": 0.0}


def test_channel_outliving_its_removal_raises(op):
    """A Python Channel held past remove_channel() must raise, not crash.

    The id lookup behind this throws std::out_of_range; letting that unwind
    through the CPython boundary would take the host process down instead of
    raising, so it is translated at the binding edge.
    """
    ch = op.create_channel("doomed")
    ch.create_keyframe(0, 0)

    op.remove_channel("doomed")

    with pytest.raises(RuntimeError):
        _ = ch.num_keyframes


def test_channel_outliving_a_clear_raises(op):
    ch = op.create_channel("doomed")
    op.clear()

    with pytest.raises(RuntimeError):
        _ = ch.name
