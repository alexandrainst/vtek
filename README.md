# vclab-vtek

vtek is a (light-weight) Vulkan library for applications to utilize the GPU for
rendering/compute.

*vtek is not under active development. For questions regarding warranties, see the [LICENSE](./LICENSE.md)*


### How to build ###

`vtek` is built with C++20, and a compiler supporting this language version must
be present.  `vtek` may be built as either a static or a dynamic-link
library. This setup is specified with CMake.  To build _everything_ on Linux
distributions (tested with Ubuntu 22):

```bash
$ cd vclab-vtek
$ mkdir build && cd build/
$ cmake ..
$ make
```

This will build `vtek` itself as a static library (the default), as well as a
number of example scripts located in `examples/` which demonstrate how to use
`vtek`.

NOTE: It is strongly suggested to do parallel builds when building `vtek`, which
can be accomplished by passing a number of threads to the `make` command: `make
-j 8` (build with 8 parallel threads).


### How to test ###

The CMake configuration adds a number of unit tests to `vtek`, in addition to
the example programs. These are built by default when CMake is configured from
the main directory of `vtek` (as described above), and otherwise ignored. This
may be overridden by client applications that import `vtek` as a CMake
subdirectory by specifying command-line arguments to CMake:

```
cd build/
cmake .. -D VTEK_EXAMPLES_CONF="" -D VTEK_UNIT_TESTS_CONF=""
```

This command will force the CMake generator to generate targets for both the
example programs and the unit tests, regardless of working directory. The
example programs are placed in the `build/` folder, and the unit tests are
placed in `build/UnitTests`. To build and run the unit tests separately:

```
make unit_tests && make test
```

NOTE: The command `make test` will only run the tests but not build them.


### Dependencies ###

`vtek` depends on a number of third-party libraries for utilities. These are
listed below:

- **Vulkan-Sdk:** May be dowloaded from LunarG's website.
- **vulkan-validationlayers-dev:** Maybe included in the `Vulkan-Sdk` package,
maybe separate install.
- **SPIRV-Tools:** Maybe included in the `Vulkan-Sdk` package, maybe separate
install.
- **glslang:** Front-end for generating SPIR-V bytecode from GLSL.
- **GLFW:** Open-source cross-platform window abstraction library.
TODO: How to include this?
- **Spdlog:** Popular open-source logging library. Contained as a git submodule.
- **GLM:** OpenGL math library, for 3d linear algebra. TODO: How to include this?
- **vma:** Vulkan memory allocation library
- **SPIRV-Reflect:** Light-weight Spir-V reflection library for shader
verification. TODO: Is this used?

The Vulkan Sdk can be found on LunarG's website (https://vulkan.lunarg.com).
Assuming Ubuntu 22.04 (instructions found on LunarG's website) and Vulkan 1.3.268:
```
wget -qO- https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo tee /etc/apt/trusted.gpg.d/lunarg.asc
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.3.268-jammy.list https://packages.lunarg.com/vulkan/1.3.268/lunarg-vulkan-1.3.268-jammy.list
sudo apt update
sudo apt install vulkan-sdk
```
Other Vulkan dependencies:
```
sudo apt install libvulkan-dev vulkan-validationlayers-dev spirv-headers spirv-tools glslang-tools glslang-dev
```
Other 3rd-party libraries:
```
sudo apt install libglm-dev libglfw3-dev
```
There are other dependencies which are contained as part of the source, but some of them are git submodules:
```
git submodule update --init --recursive
```

This should ensure that all dependencies required to build and run vtek applications are present.

### How to compile shaders ###

Shader files should be provided as pre-compiled Spir-V binaries, before running
the example programs. `vtek` comes with a number of shaders for its example
programs, all of which are located inside the `shaders` directory. There is a
Python script (requires >= Python 3.3) which automates the process of compiling
shaders. To e.g. compile the shader files inside the `shaders/simple_triangle`
directory, enter these commands in terminal: ``` cd shaders/ ./build_shaders.py
simple_triangle/ ``` This runs the shader compiler `glslangValidator` (which
comes bundled with the Vulkan Sdk) on all the individual shader files located
inside this directory.

Alternatively, the shader files can be compiled individually with either
`glsLangValidator` (by Khronos) or `glslc` (by Google). Examples:

```bash
glslangValidator --spirv-val --glsl-version 450 -S vert -V vertex.glsl -o vertex.spv
```

The `--spirv-val` also runs the Spir-V Validator (optional). The flag
`--glsl-version 450` specifies a desired version of GLSL - this should best be
provided in the shader source file as `#version 450`, in which can this flag may
be omitted. The flag `-S vert` speficies the target shader stage, and `-o
vertex.spv` the name of the output. `vtek` expects a vertex shader file to be
named `vertex.spv`, and will log whenever such a file is not found.

```bash
glslc -fshader-stage=vert test_vertex.glsl -o test_vertex.spv
```

This compiler is more similar to `GCC` and `Clang` in what the flags are called,
but ultimately they do the same things.

Raw shader files, in `GLSL` format, may also be provided to vtek. But this is
less efficient, as they have to be compiled each time a program is run. So
pre-compiling to Spir-V is preferrable.


#### Shader filenames ####

For more consistency, `vtek` places certain restrictions on the names of shader
files and how they are stored. Shaders are expected to be stored in individual
files, ie. one file for the vertex shader, one file for the fragment shader,
etc. The shader files that together creates a _program_ must be stored inside
the same directory, and must follow these naming conventions:

```
graphics_shader_dir/
├─ vertex.glsl
├─ tess_control.glsl
├─ tess_eval.glsl
├─ geometry.glsl
├─ fragment.glsl

compute_shader_dir/
├─ compute.glsl

ray_tracing_shader_dir/
├─ TODO: Filenames...
```

### How to contribute ###

As a rule of thumb, features in development should be placed on a separate
branch. The `master` branch must always compile with no warnings on all targeted
platforms, and the following rules govern use of this branch:

- The code must always compile with no warnings, and all examples should run
before committing.
- The code must never contain any out-commented code (unless thorough
explanation justifies it).
- Try to minimize the amount of "`TODO`"'s left in the code.
- Any significantly complicated code parts must be documented (documentation is
generated afterwards).
- Any significantly complicated and **testable** features must have a unit test
in place.

The folder `tools/scripts/<OS>/` contains useful scripts that may e.g. print any
leftover "`TODO`"'s in the code in a nicely formatted way. Certain features,
e.g. of graphical nature, cannot easily be tested by automated scripting, and in
such cases a test program _should_ be added to the `examples/` folder.

### LICENSE ###

vtek uses the CC BY-NC 4.0 license, which can be found [here](./LICENSE.md). For inquiries regarding commercial use, contact the Alexandra Institute. See [alexandra.dk](https://alexandra.dk/) for contact info.
