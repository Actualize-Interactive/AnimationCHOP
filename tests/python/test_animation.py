"""The operator itself: channel collection, bounds and whole-animation state."""

import pytest

import animationchop


# --- channel management -----------------------------------------------------

def test_starts_empty(op):
    assert op.num_channels == 0
    assert op.channel_names == []


def test_create_channel(op):
    ch = op.create_channel("tx")
    assert ch.name == "tx"
    assert op.num_channels == 1
    assert op.channel_names == ["tx"]


def test_create_channel_at_index(op):
    op.create_channel("a")
    op.create_channel("b")
    op.create_channel("mid", 1)
    assert op.channel_names == ["a", "mid", "b"]


def test_has_channel(op):
    op.create_channel("tx")
    assert op.has_channel("tx") is True
    assert op.has_channel("ty") is False


def test_get_channel_by_name_and_index(op):
    op.create_channel("tx")
    op.create_channel("ty")
    assert op.get_channel("ty").name == "ty"
    assert op.get_channel(0).name == "tx"


def test_remove_channel_by_name(op):
    op.create_channel("tx")
    op.create_channel("ty")
    op.remove_channel("tx")
    assert op.channel_names == ["ty"]


def test_remove_channel_by_index(op):
    op.create_channel("tx")
    op.create_channel("ty")
    op.remove_channel(0)
    assert op.channel_names == ["ty"]


def test_channels_returns_every_channel(op):
    op.create_channel("a")
    op.create_channel("b")
    assert [c.name for c in op.channels] == ["a", "b"]


def test_clear_removes_everything(op):
    op.create_channel("a")
    op.create_channel("b")
    op.clear()
    assert op.num_channels == 0
    assert op.channel_names == []


# --- bounds -----------------------------------------------------------------

def test_animation_range_is_independent_of_channel_content(op):
    """The operator's range is its own setting, not the span of its channels.

    Adding a channel reaching frame 60 leaves the range at its 0..30 default,
    so the range has to be set deliberately.
    """
    long = op.create_channel("long")
    long.create_keyframe(5, 0)
    long.create_keyframe(60, 1)

    assert long.end_time == pytest.approx(60.0)
    assert op.end_time == pytest.approx(30.0)


def test_animation_range_is_settable(op):
    op.start_time = 10.0
    op.end_time = 70.0
    assert op.start_time == pytest.approx(10.0)
    assert op.end_time == pytest.approx(70.0)
    assert op.length == pytest.approx(60.0)


def test_clear_leaves_the_range_alone(op):
    """clear() removes channels but keeps the configured range."""
    op.start_time = 10.0
    op.end_time = 70.0
    op.create_channel("tx")

    op.clear()

    assert op.num_channels == 0
    assert op.start_time == pytest.approx(10.0)
    assert op.end_time == pytest.approx(70.0)


def test_animation_num_samples_is_empty_without_channels(op):
    assert op.num_samples == 0


def test_animation_num_samples_covers_the_range(ramp, op):
    # 0..30 at 60 fps, inclusive of both ends.
    assert op.num_samples == 30 * 60 + 1


def test_channel_num_samples_takes_a_sample_rate(ramp):
    """Channel.num_samples(rate) is a method because it is rate-dependent,
    unlike the operator's num_samples, which reads the configured range."""
    assert ramp.num_samples(60.0) == 30 * 60 + 1
    assert ramp.num_samples(30.0) == 30 * 30 + 1


# --- state ------------------------------------------------------------------

def test_state_shape(ramp, op):
    state = op.state
    assert set(state) == {
        "channels", "start_time", "end_time", "length", "num_channels",
    }
    assert len(state["channels"]) == 1


def test_state_round_trips(op):
    tx = op.create_channel("tx")
    tx.create_keyframe(0, 0)
    tx.create_keyframe(30, 100)
    ty = op.create_channel("ty")
    ty.create_keyframe(0, 5)
    ty.create_keyframe(10, 15)

    saved = op.state
    before = (op.channel_names, tx.evaluate(15), ty.evaluate(5))

    op.clear()
    assert op.num_channels == 0

    op.state = saved

    assert op.channel_names == before[0]
    assert op.get_channel("tx").evaluate(15) == pytest.approx(before[1])
    assert op.get_channel("ty").evaluate(5) == pytest.approx(before[2])


def test_set_state_replaces_existing_channels(op):
    src = op.create_channel("keep")
    src.create_keyframe(0, 0)
    saved = op.state

    op.clear()
    op.create_channel("stale")
    op.set_state(saved)

    assert op.channel_names == ["keep"]


def test_get_state_matches_property(ramp, op):
    assert op.get_state() == op.state


# --- node invalidation ------------------------------------------------------

def test_mutating_marks_the_node_dirty(op):
    animationchop.reset_dirty_count()
    ch = op.create_channel("tx")
    ch.create_keyframe(0, 0)
    assert animationchop.dirty_count() > 0


def test_reading_does_not_mark_the_node_dirty(ramp):
    animationchop.reset_dirty_count()
    _ = ramp.num_keyframes
    _ = ramp.evaluate(15)
    _ = ramp.state
    assert animationchop.dirty_count() == 0
