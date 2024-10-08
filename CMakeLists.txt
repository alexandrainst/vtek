cmake_minimum_required(VERSION 3.16...3.21)
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
# Set VTEK_UNIT_TESTS_CONF to ON if we are building vtek
# --------------------------------------------------------------------------------
# Same check as above, just for unit tests
if(NOT DEFINED VTEK_UNIT_TESTS_CONF)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        set(VTEK_UNIT_TESTS_CONF ON)
    else()
        set(VTEK_UNIT_TESTS_CONF OFF)
    endif()
endif()


# --------------------------------------------------------------------------------
# Project configuration
# --------------------------------------------------------------------------------
option(VTEK_BUILD_WARNINGS "Enable compiler warnings" ON)
option(VTEK_SHARED_LIB "Build vtek as shared library" ON)
option(VTEK_CREATE_EXAMPLES "Build example programs" ${VTEK_EXAMPLES_CONF})
option(VTEK_CREATE_UNIT_TESTS "Build unit tests" ${VTEK_UNIT_TESTS_CONF})

if(VTEK_SHARED_LIB)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
else()
    set(CMAKE_POSITION_INDEPENDENT_CODE OFF)
endif()



# --------------------------------------------------------------------------------
# Dependencies for building vtek
# --------------------------------------------------------------------------------
find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

# glslang
# NOTE: If the installed version of glslang is old, then it provides no cmake script,
# and generation will fail. This repository provides an alternative:
# https://github.com/v1993/GLSLang-cmake/
#LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#include("${CMAKE_CURRENT_LIST_DIR}/cmake/Findglslang.cmake")
#find_package(GLSLang COMPONENTS GLSLang SPIRV)
# NOTE: It's probably better, still, to install from source, or include as git submodule.
find_package(glslang CONFIG REQUIRED)

# spdlog submodule
add_subdirectory(external/spdlog REQUIRED)

# Assimp submodule
option(ASSIMP_SHARED_LIBS "Build as shared library" ON)
option(ASSIMP_BUILD_TESTS "Tests probably only useful when developing on Assimp" OFF)
option(ASSIMP_INSTALL "Will install Assimp library, but we use submodule instead" OFF)
# More options: https://github.com/assimp/assimp/blob/master/Build.md
add_subdirectory(external/assimp ASSIMP_REQUIRED)

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

# Vulkan Memory Allocator (vma) as independent shared/static lib
set (vma_src
    external/vma/vk_mem_alloc.h
    external/vma/vk_mem_alloc.cpp
)

if(VTEK_SHARED_LIB)
    add_library(vma SHARED ${vma_src})
else()
    add_library(vma STATIC ${vma_src})
endif()
set_target_properties(vma PROPERTIES CXX_STANDARD 20)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # NOTE: building vma yields some warnings, just disable them here because they
    # are annoying to look at, and because the devs _probably_ know what they are doing.
    target_compile_options(
        vma PRIVATE -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter
        -Wno-unused-variable -ggdb3)
endif()

# stb libraries, notably stb_image {^_^}, similar setup as for vma
set (stb_src
    external/nothings/stb_image.h
    external/nothings/stb_image.cpp
)
add_library(stb SHARED ${stb_src})
set_target_properties(stb PROPERTIES CXX_STANDARD 20)



# --------------------------------------------------------------------------------
# Source files and build configuration for vtek
# --------------------------------------------------------------------------------
set (src
    external/spirv_reflect/spirv_reflect.h
    external/spirv_reflect/spirv_reflect.cpp

    include/vtek/api.hpp
    include/vtek/version.hpp
    include/vtek/vtek.hpp
    include/vtek/vtek_allocator.hpp
    include/vtek/vtek_application_window.hpp
    include/vtek/vtek_buffer.hpp
    include/vtek/vtek_camera.hpp
    include/vtek/vtek_command_buffer.hpp
    include/vtek/vtek_command_pool.hpp
    include/vtek/vtek_command_scheduler.hpp
    include/vtek/vtek_commands.hpp
    include/vtek/vtek_descriptor_pool.hpp
    include/vtek/vtek_descriptor_set.hpp
    include/vtek/vtek_descriptor_set_layout.hpp
    include/vtek/vtek_descriptor_type.hpp
    include/vtek/vtek_device.hpp
    include/vtek/vtek_fileio.hpp
    include/vtek/vtek_framebuffer.hpp
    include/vtek/vtek_format_support.hpp
    include/vtek/vtek_glm_includes.hpp
    include/vtek/vtek_graphics_pipeline.hpp
    include/vtek/vtek_image.hpp
    include/vtek/vtek_input.hpp
    include/vtek/vtek_instance.hpp
    include/vtek/vtek_logging.hpp
    include/vtek/vtek_main.hpp
    include/vtek/vtek_models.hpp
    include/vtek/vtek_object_handles.hpp
    include/vtek/vtek_physical_device.hpp
    include/vtek/vtek_push_constants.hpp
    include/vtek/vtek_queue.hpp
    include/vtek/vtek_render_pass.hpp
    include/vtek/vtek_sampler.hpp
    include/vtek/vtek_shaders.hpp
    include/vtek/vtek_submit_info.hpp
    include/vtek/vtek_swapchain.hpp
    include/vtek/vtek_types.hpp
    include/vtek/vtek_uniform_data.hpp
    include/vtek/vtek_vertex_data.hpp
    include/vtek/vtek_vulkan_types.hpp
    include/vtek/vtek_vulkan_version.hpp

    src/imgutils/vtek_image_load.hpp
    src/impl/vtek_command_buffer_struct.hpp
    src/impl/vtek_descriptor_set_struct.hpp
    src/impl/vtek_glfw_backend.hpp
    src/impl/vtek_init.hpp
    src/impl/vtek_queue_struct.hpp
    src/impl/vtek_vma_helpers.hpp
    src/glsl/vtek_glsl_shader_utils.hpp

    src/imgutils/vtek_image_load.cpp
    src/glsl/vtek_glsl_shader_utils.cpp
    src/vtek_allocator.cpp
    src/vtek_application_window.cpp
    src/vtek_buffer.cpp
    src/vtek_camera.cpp
    src/vtek_command_buffer.cpp
    src/vtek_command_pool.cpp
    src/vtek_command_scheduler.cpp
    src/vtek_commands.cpp
    src/vtek_descriptor_pool.cpp
    src/vtek_descriptor_set.cpp
    src/vtek_descriptor_set_layout.cpp
    src/vtek_descriptor_type.cpp
    src/vtek_device.cpp
    src/vtek_fileio.cpp
    src/vtek_framebuffer.cpp
    src/vtek_format_support.cpp
    src/vtek_graphics_pipeline.cpp
    src/vtek_image.cpp
    src/vtek_instance.cpp
    src/vtek_logging.cpp
    src/vtek_main.cpp
    src/vtek_models.cpp
    src/vtek_physical_device.cpp
    src/vtek_queue.cpp
    src/vtek_render_pass.cpp
    src/vtek_sampler.cpp
    src/vtek_shaders.cpp
    src/vtek_swapchain.cpp
    src/vtek_uniform_data.cpp
    src/vtek_vertex_data.cpp
    src/vtek_vulkan_types.cpp
    )

if(VTEK_SHARED_LIB)
    message(STATUS "Build vtek as shared library")
    add_library(vtek SHARED ${src})
else()
    message(STATUS "Build vtek as static library")
    add_library(vtek STATIC ${src})
endif()
unset(VTEK_SHARED_LIB CACHE) # remove from cache to force reset each time we run cmake

add_library(vtek::vtek ALIAS vtek)

set_target_properties(vtek PROPERTIES CXX_STANDARD 20)
target_include_directories(vtek PRIVATE
    ${PROJECT_SOURCE_DIR}/include/vtek
    ${PROJECT_SOURCE_DIR}/external/nothings
    ${PROJECT_SOURCE_DIR}/external/spirv_reflect
    ${PROJECT_SOURCE_DIR}/external/vma
    ${PROJECT_SOURCE_DIR}/pchs
)
target_include_directories(vtek PUBLIC
    ${PROJECT_SOURCE_DIR}/external/assimp/include
    ${PROJECT_SOURCE_DIR}/external/spdlog/include
    ${PROJECT_SOURCE_DIR}/include
    ${Vulkan_INCLUDE_DIRS}
)
target_precompile_headers(vtek PRIVATE
    ${PROJECT_SOURCE_DIR}/pchs/vtek_vulkan.pch
)
target_compile_definitions(vtek PRIVATE SPDLOG_COMPILED_LIB)

if(VTEK_BUILD_WARNINGS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options( vtek PRIVATE -Wall -Wextra -Wno-unused-parameter -ggdb3) # -Werror
    endif()
endif()

target_link_libraries(vtek LINK_PUBLIC
    spdlog::spdlog glfw Vulkan::Vulkan vma stb assimp::assimp
    glslang::glslang glslang::SPIRV glslang::glslang-default-resource-limits)


# --------------------------------------------------------------------------------
# Conditionally build example programs
# --------------------------------------------------------------------------------
if(VTEK_CREATE_EXAMPLES)
    message(STATUS "Generate example programs")
    set(examples
        examples/01_triangle.cpp
        examples/02_triangle_move.cpp
        examples/03_vertex_buffer.cpp
        examples/04_dynamic_circle.cpp
        examples/05_camera.cpp
        examples/06_model.cpp
        examples/07_textured_model.cpp
        examples/08_framebuffer.cpp
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


# --------------------------------------------------------------------------------
# Confitionally build unit tests
# --------------------------------------------------------------------------------
if(VTEK_CREATE_UNIT_TESTS)
    message(STATUS "Generate unit tests")
    set(unit_test_src
        tests/test_camera.cpp
        tests/test_shaders.cpp
        # tests/test_formats.cpp
        # tests/test_ut.cpp
    )

    # Display terminal output on failed tests
    list(APPEND CMAKE_CTEST_ARGUMENTS "--output-on-failure")

    enable_testing()
    add_custom_target(unit_tests)

    # Output tests to custom "UnitTests" subfolder
    set(VTEK_TEMP_RUNTIME_OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/UnitTests)

    foreach(utpath ${unit_test_src})
        cmake_path(GET utpath STEM utname)
        add_executable(${utname} ${utpath})
        set_target_properties(${utname} PROPERTIES CXX_STANDARD 20)
        target_link_libraries(${utname} LINK_PUBLIC vtek::vtek)
        target_precompile_headers(${utname} PRIVATE
            ${PROJECT_SOURCE_DIR}/pchs/vtek_vulkan.pch
        )
        target_compile_definitions(${utname} PRIVATE SPDLOG_COMPILED_LIB)
        target_link_directories(${utname} PUBLIC ${CMAKE_OUTPUT_DIR})
        target_compile_options(${utname} PRIVATE -ggdb3)
        target_include_directories(${utname} PRIVATE
            ${PROJECT_SOURCE_DIR}/external
            ${PROJECT_SOURCE_DIR}/external/vma
            ${PROJECT_SOURCE_DIR}/include/vtek
            ${PROJECT_SOURCE_DIR}/pchs
        )

        add_test(NAME ${utname} COMMAND ${utname})
        add_dependencies(unit_tests ${utname})
    endforeach()

    # Restore output directory in case it's needed after this section
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VTEK_TEMP_RUNTIME_OUTPUT_DIR})
endif()
