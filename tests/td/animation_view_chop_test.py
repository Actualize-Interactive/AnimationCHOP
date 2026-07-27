"""In-TouchDesigner test suite for AnimationViewCHOP.

AnimationViewCHOP has no Python API of its own -- it reads another operator's
animation and republishes it as CHOP data for UI to build on. So unlike
AnimationCHOP, none of it can be covered by the headless pytest suite: every
assertion here is about what the node cooks.

The five view modes each produce a different table, and the checks below are
mostly about shape and correspondence: that the channel names are the documented
ones, that there is one sample per keyframe / segment / channel as appropriate,
and that the values agree with the source animation rather than merely being
present.

Loaded as a DAT under /local/modules and driven by td_test_runner, which passes
both operators in. Importing this module runs nothing.
"""

# The channel tables each view mode publishes, in order. Hard-coded rather than
# read back from the node: these names are the operator's public surface, and a
# reordering or rename would break every UI built on it, so the test should fail
# when they change.
KEYFRAME_CHANS = [
    'channel_index', 'keyframe_index', 'time', 'value',
    'in_handle_time', 'in_handle_value', 'out_handle_time', 'out_handle_value',
    'function', 'handle_mode', 'selected', 'display',
]

SEGMENT_CHANS = [
    'channel_index', 'segment_index', 'start_time', 'start_value',
    'end_time', 'end_value', 'start_handle_time', 'start_handle_value',
    'end_handle_time', 'end_handle_value', 'display_start_handle',
    'display_end_handle', 'selected', 'selected_start_handles',
    'selected_end_handles',
]

CHANNEL_CHANS = [
    'num_keyframes', 'start_time', 'end_time', 'start_index',
    'selected', 'display',
]

ANIMATION_CHANS = [
    'num_channels', 'min_keyframe_time', 'max_keyframe_time',
    'min_keyframe_value', 'max_keyframe_value',
]

VIEW_RATE = 60.0
VIEW_START = 0.0
VIEW_END = 2.0


def _names(chop):
    return [c.name for c in chop.chans()]


def _vals(chop, name):
    """A CHOP channel's samples as a plain list.

    td.Channel supports indexing but not iteration, so list() on one raises
    TypeError rather than giving its samples.
    """
    chan = chop[name]
    return [chan[i] for i in range(chop.numSamples)]


def build_fixture(anim_chop):
    """A small animation with a known shape, used by every view mode.

    Two channels of three keyframes each, so there are 6 keyframes and 4
    segments, and the two differ in range so the per-channel numbers cannot
    quietly come from the wrong channel.
    """
    anim_chop.clear()
    anim_chop.par.Outputmode = 'range'
    anim_chop.par.Indexunit = 'seconds'
    anim_chop.par.Timeslice = 0
    anim_chop.par.Samplerate = VIEW_RATE
    anim_chop.par.Range1 = VIEW_START
    anim_chop.par.Range2 = VIEW_END

    a = anim_chop.create_channel('view_a')
    a.create_keyframe(0.0, 0.0)
    a.create_keyframe(1.0, 10.0)
    a.create_keyframe(2.0, -5.0)

    b = anim_chop.create_channel('view_b')
    b.create_keyframe(0.5, 3.0)
    b.create_keyframe(1.0, 4.0)
    b.create_keyframe(1.5, 25.0)


def configure(view_chop, mode):
    view_chop.par.Viewmode = mode


def configure_samples(view_chop):
    view_chop.par.Viewmode = 'samples'
    view_chop.par.Rangeunit = 'seconds'
    view_chop.par.Samplerate = VIEW_RATE
    view_chop.par.Range1 = VIEW_START
    view_chop.par.Range2 = VIEW_END


# --- one check function per view mode ---------------------------------------
#
# Each runs after the node has cooked in that mode; td_test_runner steps through
# them a few frames apart.

def check_samples_view(anim_chop, view_chop, result):
    result.begin_suite('view: samples')
    print("\n--- Testing AnimationViewCHOP samples view ---")

    try:
        result.assert_equal(anim_chop.num_channels, view_chop.numChans,
                            "Samples view has one CHOP channel per animation channel")
        result.assert_equal(['view_a', 'view_b'], _names(view_chop),
                            "Samples view channel names come from the animation")

        # Half-open, matching AnimationCHOP: a 2-second span at 60 Hz is 120.
        expected = int(round((VIEW_END - VIEW_START) * VIEW_RATE))
        result.assert_equal(expected, view_chop.numSamples,
                            "Samples view covers the range at the sample rate")
    except Exception as e:
        result.record_exception("Samples view shape", e)
        return

    try:
        src = anim_chop.get_channel('view_a')
        out = view_chop['view_a']

        mismatches = 0
        worst = 0.0
        for i in range(view_chop.numSamples):
            t = VIEW_START + i / VIEW_RATE
            delta = abs(out[i] - src.evaluate(t))
            worst = max(worst, delta)
            if delta > 1e-3:
                mismatches += 1
        result.assert_equal(0, mismatches,
                            f"Every samples-view sample matches Channel.evaluate() "
                            f"(worst delta {worst:.6f})")

        # Half-open: the last sample sits one period short of the range end.
        result.assert_near(src.evaluate(VIEW_END - 1 / VIEW_RATE),
                           out[view_chop.numSamples - 1], 1e-3,
                           "Samples view stops one period short of the range end")
    except Exception as e:
        result.record_exception("Samples view values", e)


def check_keyframes_view(anim_chop, view_chop, result):
    result.begin_suite('view: keyframes')
    print("\n--- Testing AnimationViewCHOP keyframes view ---")

    try:
        result.assert_equal(KEYFRAME_CHANS, _names(view_chop),
                            "Keyframes view publishes the documented channels")
        result.assert_equal(6, view_chop.numSamples,
                            "Keyframes view has one sample per keyframe across all channels")
    except Exception as e:
        result.record_exception("Keyframes view shape", e)
        return

    try:
        # Rows are laid out channel by channel, so the first three are view_a.
        result.assert_equal([0.0, 0.0, 0.0, 1.0, 1.0, 1.0],
                            _vals(view_chop, 'channel_index'),
                            "Keyframes view channel_index groups by channel")
        result.assert_equal([0.0, 1.0, 2.0, 0.0, 1.0, 2.0],
                            _vals(view_chop, 'keyframe_index'),
                            "Keyframes view keyframe_index restarts per channel")

        a = anim_chop.get_channel('view_a')
        times = _vals(view_chop, 'time')
        values = _vals(view_chop, 'value')
        for k in range(3):
            result.assert_near(a.keyframe(k).time, times[k], 1e-4,
                               f"Keyframes view time matches keyframe {k}")
            result.assert_near(a.keyframe(k).value, values[k], 1e-4,
                               f"Keyframes view value matches keyframe {k}")

        result.assert_near(a.keyframe(1).in_handle.time,
                           view_chop['in_handle_time'][1], 1e-4,
                           "Keyframes view in_handle_time matches the keyframe")
        result.assert_near(a.keyframe(1).out_handle.value,
                           view_chop['out_handle_value'][1], 1e-4,
                           "Keyframes view out_handle_value matches the keyframe")
        result.assert_equal(float(int(a.keyframe(0).function)),
                            view_chop['function'][0],
                            "Keyframes view function matches the keyframe")
        result.assert_equal(float(int(a.keyframe(0).handle_mode)),
                            view_chop['handle_mode'][0],
                            "Keyframes view handle_mode matches the keyframe")
    except Exception as e:
        result.record_exception("Keyframes view values", e)


def check_segments_view(anim_chop, view_chop, result):
    result.begin_suite('view: segments')
    print("\n--- Testing AnimationViewCHOP segments view ---")

    try:
        result.assert_equal(SEGMENT_CHANS, _names(view_chop),
                            "Segments view publishes the documented channels")
        # Three keyframes per channel is two segments, so four in total.
        result.assert_equal(4, view_chop.numSamples,
                            "Segments view has one sample per keyframe gap")
    except Exception as e:
        result.record_exception("Segments view shape", e)
        return

    try:
        a = anim_chop.get_channel('view_a')
        result.assert_near(a.keyframe(0).time, view_chop['start_time'][0], 1e-4,
                           "Segments view start_time is the segment's first keyframe")
        result.assert_near(a.keyframe(1).time, view_chop['end_time'][0], 1e-4,
                           "Segments view end_time is the segment's second keyframe")
        result.assert_near(a.keyframe(1).value, view_chop['end_value'][0], 1e-4,
                           "Segments view end_value is the segment's second keyframe")
        result.assert_equal([0.0, 0.0, 1.0, 1.0],
                            _vals(view_chop, 'channel_index'),
                            "Segments view channel_index groups by channel")
        result.assert_equal([0.0, 1.0, 0.0, 1.0],
                            _vals(view_chop, 'segment_index'),
                            "Segments view segment_index restarts per channel")
    except Exception as e:
        result.record_exception("Segments view values", e)


def check_channels_view(anim_chop, view_chop, result):
    result.begin_suite('view: channels')
    print("\n--- Testing AnimationViewCHOP channels view ---")

    try:
        result.assert_equal(CHANNEL_CHANS, _names(view_chop),
                            "Channels view publishes the documented channels")
        result.assert_equal(2, view_chop.numSamples,
                            "Channels view has one sample per animation channel")
    except Exception as e:
        result.record_exception("Channels view shape", e)
        return

    try:
        result.assert_equal([3.0, 3.0], _vals(view_chop, 'num_keyframes'),
                            "Channels view num_keyframes matches each channel")
        result.assert_near(0.0, view_chop['start_time'][0], 1e-4,
                           "Channels view start_time is the first keyframe's time")
        result.assert_near(2.0, view_chop['end_time'][0], 1e-4,
                           "Channels view end_time is the last keyframe's time")
        result.assert_near(0.5, view_chop['start_time'][1], 1e-4,
                           "Channels view start_time is per channel, not shared")

        # start_index is the running offset into the keyframes view, which is
        # what lets a UI slice that table per channel.
        result.assert_equal([0.0, 3.0], _vals(view_chop, 'start_index'),
                            "Channels view start_index accumulates keyframe counts")
        result.assert_equal([1.0, 1.0], _vals(view_chop, 'display'),
                            "Channels view display defaults on")
    except Exception as e:
        result.record_exception("Channels view values", e)


def check_animation_view(anim_chop, view_chop, result):
    result.begin_suite('view: animation')
    print("\n--- Testing AnimationViewCHOP animation view ---")

    try:
        result.assert_equal(ANIMATION_CHANS, _names(view_chop),
                            "Animation view publishes the documented channels")
        result.assert_equal(1, view_chop.numSamples,
                            "Animation view is a single sample")
    except Exception as e:
        result.record_exception("Animation view shape", e)
        return

    try:
        result.assert_equal(2.0, view_chop['num_channels'][0],
                            "Animation view num_channels counts the channels")
        # Bounds are across every keyframe of every channel: view_a spans
        # 0..2 and -5..10, view_b spans 0.5..1.5 and 3..25.
        result.assert_near(0.0, view_chop['min_keyframe_time'][0], 1e-4,
                           "Animation view min_keyframe_time spans all channels")
        result.assert_near(2.0, view_chop['max_keyframe_time'][0], 1e-4,
                           "Animation view max_keyframe_time spans all channels")
        result.assert_near(-5.0, view_chop['min_keyframe_value'][0], 1e-4,
                           "Animation view min_keyframe_value spans all channels")
        result.assert_near(25.0, view_chop['max_keyframe_value'][0], 1e-4,
                           "Animation view max_keyframe_value spans all channels")
    except Exception as e:
        result.record_exception("Animation view values", e)


# --- empty channels ---------------------------------------------------------

def setup_empty_channel(anim_chop, view_chop):
    """An empty channel alongside a populated one.

    A channel with no keyframes has no segments, and the segment count was
    computed as size() - 1 on an unsigned type -- so an empty channel wrapped to
    SIZE_MAX and undercounted the segment table, which the fill loop then wrote
    past the end of. Creating a channel before keying it is completely ordinary,
    so this configuration has to cook.
    """
    anim_chop.clear()
    populated = anim_chop.create_channel('has_keys')
    populated.create_keyframe(0.0, 0.0)
    populated.create_keyframe(1.0, 1.0)
    populated.create_keyframe(2.0, 0.0)
    anim_chop.create_channel('empty')          # no keyframes
    anim_chop.create_channel('single').create_keyframe(0.0, 5.0)

    view_chop.par.Viewmode = 'segments'


def check_empty_channel(anim_chop, view_chop, result):
    result.begin_suite('view: empty channels')
    print("\n--- Testing AnimationViewCHOP with empty channels ---")

    try:
        # Reaching here at all is most of the test: the bug crashed the cook.
        result.assert_equal(2, view_chop.numSamples,
                            "Segments view counts only the populated channel's gaps")
        result.assert_equal([0.0, 0.0], _vals(view_chop, 'channel_index'),
                            "Segments view attributes both segments to the keyed channel")
        result.assert_equal(3, anim_chop.num_channels,
                            "The empty and single-keyframe channels still exist")
    except Exception as e:
        result.record_exception("Empty channel handling", e)


_saved_source = None


def setup_no_source(view_chop):
    """Clear the source operator -- the state a freshly created node is in.

    setDataInstance() fails without one and execute() returns having written
    nothing, so every sample used to keep whatever was in TouchDesigner's
    buffer. A node that has not been hooked up yet should read as zeros, not
    NaN.
    """
    global _saved_source
    _saved_source = view_chop.par.Datasource.eval()
    view_chop.par.Datasource = ''


def check_no_source(view_chop, result):
    result.begin_suite('view: no source')
    print("\n--- Testing AnimationViewCHOP with no source operator ---")

    try:
        bad = []
        for c in range(view_chop.numChans):
            chan = view_chop[c]
            for i in range(view_chop.numSamples):
                if chan[i] != chan[i] or abs(chan[i]) > 1e30:
                    bad.append((chan.name, i))
                    break
        result.assert_equal([], bad,
                            "No NaN in cooked output with no source operator")
    except Exception as e:
        result.record_exception("No-source handling", e)
    finally:
        try:
            if _saved_source is not None:
                view_chop.par.Datasource = _saved_source
        except Exception:
            pass


def check_empty_animation(anim_chop, view_chop, result):
    """No channels at all -- the state a freshly created node is in."""
    result.begin_suite('view: empty animation')
    print("\n--- Testing AnimationViewCHOP with no channels ---")

    try:
        result.assert_true(view_chop.numSamples >= 0,
                           "An animation with no channels cooks without error")
        # OP.error was deprecated in favour of OP.errors(); reading the old
        # attribute raises rather than returning an empty string.
        result.assert_equal('', view_chop.errors(),
                            "An animation with no channels raises no error")
    except Exception as e:
        result.record_exception("Empty animation handling", e)
