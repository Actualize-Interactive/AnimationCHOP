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


def test_range_setters_push_the_opposite_bound(op):
    """Setting one bound past the other drags that one along, never inverts.

    This matters because the setters are assigned one at a time: if start
    merely clamped, moving a whole range forward would silently land on the
    wrong start for as long as the old end held it back.
    """
    op.start_time = 0.0
    op.end_time = 30.0

    op.start_time = 40.0
    assert (op.start_time, op.end_time) == pytest.approx((40.0, 40.0))

    op.end_time = 10.0
    assert (op.start_time, op.end_time) == pytest.approx((10.0, 10.0))


def test_range_can_be_moved_forward_in_either_order(op):
    """A consequence of the above: neither assignment order loses the range."""
    op.start_time = 0.0
    op.end_time = 30.0

    op.start_time = 40.0
    op.end_time = 70.0
    assert (op.start_time, op.end_time) == pytest.approx((40.0, 70.0))

    op.start_time = 0.0
    op.end_time = 30.0

    op.end_time = 70.0
    op.start_time = 40.0
    assert (op.start_time, op.end_time) == pytest.approx((40.0, 70.0))


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
    """The range is half-open: 30 seconds at 60 fps is 1800 samples.

    The end time is not sampled, so a span of n sample periods gives n
    samples -- what a CHOP means by a sample count, and what the operator's
    output length is.
    """
    assert op.num_samples == 30 * 60


def test_channel_num_samples_takes_a_sample_rate(ramp):
    """Channel.num_samples(rate) is a method because it is rate-dependent,
    unlike the operator's num_samples, which reads the configured range."""
    assert ramp.num_samples(60.0) == 30 * 60
    assert ramp.num_samples(30.0) == 30 * 30


def test_num_samples_is_a_whole_number_of_periods(op):
    """A duration that lands a few ulps off a whole number must not gain a
    sample -- the count is rounded to the nearest whole period first."""
    op.start_time = 0.0
    op.end_time = 4.0
    ch = op.create_channel("tx")
    ch.create_keyframe(0, 0)
    ch.create_keyframe(4, 1)

    assert ch.num_samples(30.0) == 120
    assert op.num_samples == 4 * 60


def test_evaluate_range_by_rate_is_half_open(ramp):
    """The samples are one period apart and stop short of the end time."""
    values = ramp.evaluate_range_by_rate(0.0, 30.0, 60.0)

    assert len(values) == 30 * 60
    assert values[0] == pytest.approx(0.0)
    # Last sample sits one period before the end, not on it.
    assert values[-1] == pytest.approx(ramp.evaluate(30.0 - 1.0 / 60.0))


def test_evaluate_range_is_half_open_by_default(ramp):
    """Both sampling methods default to half-open, and agree with each other."""
    values = ramp.evaluate_range(0.0, 30.0, 30 * 60)

    assert len(values) == 30 * 60
    assert values[0] == pytest.approx(0.0)
    assert values[-1] == pytest.approx(ramp.evaluate(30.0 - 1.0 / 60.0))
    assert values == pytest.approx(ramp.evaluate_range_by_rate(0.0, 30.0, 60.0))


def test_range_end_inclusive_lands_on_the_end(op, ramp):
    """RangeEnd.INCLUSIVE samples the end time -- for plots and lookup tables."""
    values = ramp.evaluate_range(0.0, 30.0, 121, op.RangeEnd.INCLUSIVE)

    assert len(values) == 121
    assert values[0] == pytest.approx(0.0)
    assert values[-1] == pytest.approx(ramp.evaluate(30.0))


def test_range_end_inclusive_adds_the_closing_sample_by_rate(op, ramp):
    """By rate, INCLUSIVE is one more sample: the one landing on the end."""
    exclusive = ramp.evaluate_range_by_rate(0.0, 30.0, 60.0)
    inclusive = ramp.evaluate_range_by_rate(0.0, 30.0, 60.0, op.RangeEnd.INCLUSIVE)

    assert len(inclusive) == len(exclusive) + 1
    assert inclusive[:-1] == pytest.approx(exclusive)
    assert inclusive[-1] == pytest.approx(ramp.evaluate(30.0))


def test_num_samples_follows_range_end(op, ramp):
    assert ramp.num_samples(60.0) == 30 * 60
    assert ramp.num_samples(60.0, op.RangeEnd.INCLUSIVE) == 30 * 60 + 1


def test_range_end_rejects_nonsense(op, ramp):
    with pytest.raises(ValueError):
        ramp.evaluate_range(0.0, 30.0, 10, 99)
    with pytest.raises(TypeError):
        ramp.evaluate_range(0.0, 30.0, 10, "inclusive")


def test_half_open_ranges_join_without_repeating(op):
    """The reason the default is half-open: adjacent spans do not double up.

    Sampling 0..1 and 1..2 back to back must give the same values as sampling
    0..2 in one go -- no repeated sample at the seam.
    """
    ch = op.create_channel("seam")
    ch.create_keyframe(0, 0)
    ch.create_keyframe(2, 100)

    joined = (ch.evaluate_range_by_rate(0.0, 1.0, 60.0)
              + ch.evaluate_range_by_rate(1.0, 2.0, 60.0))

    assert joined == pytest.approx(ch.evaluate_range_by_rate(0.0, 2.0, 60.0))


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
