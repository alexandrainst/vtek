cmake_minimum_required(VERSION 3.10...3.21)
project(vtek CXX)


# --------------------------------------------------------------------------------
# Set VTEK_EXAMPLES_CONF to ON if we are building vtek
# --------------------------------------------------------------------------------
# Check if vtek is being used directly, or via add_subdirectory for another project
if(NOT DEFINED VTEK_EXAMPLES_CONF)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(VTEK_EXAMPLES_CONF ON)
  else()
	set(VTEK_EXAMPLES_CONF OFF)
  endif()
endif()


# --------------------------------------------------------------------------------
# Project configuration
# --------------------------------------------------------------------------------
option(VTEK_BUILD_WARNINGS "Enable compiler warnings" ON)
option(VTEK_SHARED_LIB "Build vtek as shared library" OFF)
option(VTEK_CREATE_EXAMPLES "Build example programs" ${VTEK_EXAMPLES_CONF})

set(CMAKE_POSITION_INDEPENDENT_CODE OFF)


# --------------------------------------------------------------------------------
# Dependencies for building vtek
# --------------------------------------------------------------------------------
find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)
find_package(glslang REQUIRED)

# spdlog submodule
add_subdirectory(external/spdlog REQUIRED)

# glm as either local installation or CMake subcontent
find_package(glm QUIET)
if (NOT ${glm_FOUND})
  message("No local installation of GLM found - downloading through CMake")
  include(FetchContent)
  FetchContent_Declare(libglm
      URL https://github.com/g-truc/glm/releases/download/0.9.9.8/glm-0.9.9.8.zip
      URL_HASH MD5=69895110052f0d711c9c54fbf385f6f5
  )
  FetchContent_Populate(libglm)
  set(glm_DIR ${libglm_SOURCE_DIR}/cmake/glm)
  find_package(glm)
endif()

# TODO: Conditional glfw find
# NOTE: Could possibly we written as `cmake -DUSE_GLFW=true`, or something like that
if(VTEK_CREATE_EXAMPLES)
  find_package(glfw3 REQUIRED)
endif()


set (src
  external/spirv_reflect/spirv_reflect.h
  external/spirv_reflect/spirv_reflect.cpp

  include/vtek/version.hpp
  include/vtek/vtek.hpp
  include/vtek/vtek_application_window.hpp
  include/vtek/vtek_command_buffer.hpp
  include/vtek/vtek_command_pool.hpp
  include/vtek/vtek_device.hpp
  include/vtek/vtek_fileio.hpp
  include/vtek/vtek_graphics_pipeline.hpp
  include/vtek/vtek_input.hpp
  include/vtek/vtek_instance.hpp
  include/vtek/vtek_logging.hpp
  include/vtek/vtek_main.hpp
  include/vtek/vtek_physical_device.hpp
  include/vtek/vtek_queue.hpp
  include/vtek/vtek_render_pass.hpp
  include/vtek/vtek_shaders.hpp
  include/vtek/vtek_submit_info.hpp
  include/vtek/vtek_swapchain.hpp
  include/vtek/vtek_types.hpp
  include/vtek/vtek_vertex_data.hpp
  include/vtek/vtek_vulkan_version.hpp

  src/impl/vtek_glfw_backend.hpp
  src/impl/vtek_host_allocator.hpp
  src/impl/vtek_init.hpp
  src/impl/vtek_queue_struct.hpp
  src/impl/vtek_vulkan_helpers.hpp

  src/vtek_application_window.cpp
  src/vtek_command_buffer.cpp
  src/vtek_command_pool.cpp
  src/vtek_device.cpp
  src/vtek_fileio.cpp
  src/vtek_graphics_pipeline.cpp
  src/vtek_host_allocator.cpp
  src/vtek_instance.cpp
  src/vtek_logging.cpp
  src/vtek_main.cpp
  src/vtek_physical_device.cpp
  src/vtek_queue.cpp
  src/vtek_render_pass.cpp
  src/vtek_shaders.cpp
  src/vtek_swapchain.cpp
  src/vtek_vertex_data.cpp
  )

if(VTEK_SHARED_LIB)
  message("Build vtek as shared library")
  add_library(vtek SHARED ${src})
else()
  message("Build vtek as static library")
  add_library(vtek STATIC ${src})
endif()

add_library(vtek::vtek ALIAS vtek)

set_target_properties(vtek PROPERTIES CXX_STANDARD 20)
target_include_directories(vtek PRIVATE ${PROJECT_SOURCE_DIR}/include/vtek)
target_include_directories(vtek PRIVATE ${PROJECT_SOURCE_DIR}/external/spirv_reflect)
target_include_directories(vtek PUBLIC
  external/spdlog/include ${PROJECT_SOURCE_DIR}/include ${Vulkan_INCLUDE_DIRS})
target_compile_definitions(vtek PRIVATE SPDLOG_COMPILED_LIB)

if(VTEK_BUILD_WARNINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options( vtek PRIVATE -Wall -Wextra -Wno-unused-parameter -ggdb3) # -Werror
  endif()
endif()

target_link_libraries(vtek LINK_PUBLIC
  spdlog::spdlog glfw Vulkan::Vulkan
  glslang::glslang glslang::SPIRV glslang::glslang-default-resource-limits)


# --------------------------------------------------------------------------------
# Conditionally build example programs
# --------------------------------------------------------------------------------
if(VTEK_CREATE_EXAMPLES)
  set(examples
	examples/triangle_plain.cpp
	)

  foreach(expath ${examples})
	cmake_path(GET expath STEM exname)
	add_executable(${exname} ${expath})
	set_target_properties(${exname} PROPERTIES CXX_STANDARD 20)
	target_link_libraries(${exname} LINK_PUBLIC vtek::vtek)
	target_compile_definitions(${exname} PRIVATE SPDLOG_COMPILED_LIB)
	# target_link_libraries(${exname} PUBLIC vtek::vtek spdlog::spdlog Vulkan::Vulkan)
	target_link_directories(${exname} PUBLIC ${CMAKE_OUTPUT_DIR})
	target_compile_options(${exname} PRIVATE -ggdb3)
  endforeach()

endif()
