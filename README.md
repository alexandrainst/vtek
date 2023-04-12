# vclab-vtek

vtek is a (light-weight) Vulkan library for applications to utilize the GPU for rendering/compute.

_For now this is an internally managed code repository, but open-sourcing may be desired later,
at which point a `LICENSE` file should be added documenting proper use of the code. The MIT license
is probably the best suited for this purpose (GPL licenses have compatibility restrictions)._

### How to build ###

`vtek` may be built as either a static or a dynamic-link library. This setup is specified with CMake.
To build _everything_ on Linux distributions (tested with Ubuntu 22):

```bash
$ cd vclab-vtek
$ mkdir build && cd build/
$ cmake ..
$ make
```

This will build `vtek` itself as a static library (the default), as well as a number of example
scripts located in `examples/` which demonstrate how to use `vtek`.

### Dependencies ###

`vtek` depends on a number of third-party libraries for utilities. These are listed below:

- **Vulkan-Sdk:** May be dowloaded from LunarG's website.
- **GLFW:** Open-source cross-platform window abstraction library. TODO: How to include this?
- **Spdlog:** Popular open-source logging library. Contained as a git submodule.
- **vma:** Vulkan memory allocation library


### How to contribute ###

As a rule of thumb, features in development should be placed on a separate branch. The `master`
branch must always compile with no warnings on all targeted platforms, and the following rules
govern use of this branch:

- The code must always compile with no warnings, and all examples should run before committing.
- The code must never contain any out-commented code (unless thorough explanation justifies it).
- Try to minimize the amount of "`TODO`"'s left in the code.
- Any significantly complicated code parts must be documented (documentation is generated afterwards).
- Any significantly complicated and **testable** features must have a unit test in place.

The folder `tools/scripts/<OS>/` contains useful scripts that may e.g. print any leftover "`TODO`"'s
in the code in a nicely formatted way. Certain features, e.g. of graphical nature, cannot easily be
tested by automated scripting, and in such cases a test program _should_ be added to the `examples/`
folder.
