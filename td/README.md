# Example project

`AnimationCHOP.toe` is a demonstration project for the operators, together with
`Keyframer.tox` and the Python modules and shaders it loads.

**This is a work in progress and will likely be removed from this repository.**
The Keyframer component is a project in its own right, and is expected to live
in the KeyframerComp repository rather than here. Treat what is in this folder
as a reference for how the operators are driven, not as a supported component to
build on — its layout, module names and parameters may change or disappear
without a deprecation period.

The operators themselves are stable and documented; see [`../docs`](../docs) for
the Python API and [`../README.md`](../README.md) for installation.

`Plugins/` is created by the build, which copies the compiled operators there so
this project can load them. It is not tracked.
