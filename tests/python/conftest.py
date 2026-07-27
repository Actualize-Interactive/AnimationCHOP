"""Shared fixtures for the AnimationCHOP binding tests.

The `animationchop` extension creates a single operator instance at import and
keeps it for the session -- the same lifetime it has inside TouchDesigner, where
the node owns its anim::Animation. So the animation is reset between tests
rather than rebuilt, and tests must not rely on state from one another.
"""

import pytest

import animationchop


# The operator's range defaults to 0..30 and is independent of the channels,
# so clear() does not restore it -- the fixture has to, or a test that sets the
# range leaks it into every test that follows.
DEFAULT_START_TIME = 0.0
DEFAULT_END_TIME = 30.0


def _reset(node):
    node.clear()
    node.start_time = DEFAULT_START_TIME
    node.end_time = DEFAULT_END_TIME


@pytest.fixture
def op():
    """The operator: no channels, default range, dirty counter zeroed."""
    _reset(animationchop.op)
    animationchop.reset_dirty_count()
    yield animationchop.op
    _reset(animationchop.op)


@pytest.fixture
def channel(op):
    """An empty channel named 'tx'."""
    return op.create_channel("tx")


@pytest.fixture
def ramp(op):
    """A channel ramping 0 -> 100 over frames 0 -> 30."""
    ch = op.create_channel("ramp")
    ch.create_keyframe(0, 0)
    ch.create_keyframe(30, 100)
    return ch
