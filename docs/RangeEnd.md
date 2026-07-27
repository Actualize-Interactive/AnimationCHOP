# RangeEnd

Whether a sampled range includes its end time. Passed as an optional trailing
argument to [`Channel.evaluate_range`](Channel.md),
`Channel.evaluate_range_by_rate`.

## Values

| Value | Description |
|-------|-------------|
| `RangeEnd.EXCLUSIVE` | The end time is **not** sampled; the range is `[start, end)`. The default. |
| `RangeEnd.INCLUSIVE` | The end time **is** sampled; the range is `[start, end]`. |

## Which to use

**`EXCLUSIVE` (the default)** is the timeline reading: a sample covers the
interval that follows it, and the end of a range is an edge rather than a point.
A span of n sample periods gives n samples. This is what frame- and audio-rate
hosts do, and it is what the operators' own CHOP output uses — a CHOP's samples
are implicitly one period apart, since the format stores no per-sample times.

The practical consequence is that ranges join cleanly:

```python
# Sampling 0..1 and 1..2 back to back gives exactly the same values as
# sampling 0..2 in one go -- no repeated sample at the seam.
ch.evaluate_range_by_rate(0, 1, 60) + ch.evaluate_range_by_rate(1, 2, 60)
```

With an inclusive range each cycle boundary would carry two identical values,
and every caller would have to know to drop one — including anything using
`Extend.REPEAT` or `Extend.MIRROR`.

**`INCLUSIVE`** is for when samples are points on the curve rather than spans of
time: plotting a curve, building an interpolation lookup table, integrating
numerically. Without it the last point falls one step short of the end, so a
plotted line stops before the final keyframe.

## Examples

```python
n = op('animation1')
ch = n.get_channel('tx')     # keyframed 0 .. 2 seconds

# 120 values, the last at 1.9833
ch.evaluate_range_by_rate(0, 2, 60)

# 121 values, the last exactly at 2.0
ch.evaluate_range_by_rate(0, 2, 60, n.RangeEnd.INCLUSIVE)

# The operator's own count is always the half-open one, matching what it
# outputs; RangeEnd applies to the evaluate calls, not the node's length.
n.num_samples                               # 120 over a 0..2 range at 60

# By count rather than by rate, the two differ in spacing, not length:
# both return 100 values, but INCLUSIVE lands the last one on the end.
ch.evaluate_range(0, 2, 100)
ch.evaluate_range(0, 2, 100, n.RangeEnd.INCLUSIVE)
```
