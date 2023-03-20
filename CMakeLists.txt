cmake_minimum_required(VERSION 3.10)
project(vtek)

# spdlog submodule
add_subdirectory(external/spdlog)

# Project configuration
option(VTEK_BUILD_WARNINGS "Enable compiler warnings" ON)
option(VTEK_SHARED_LIB "Build vtek as shared library" OFF)

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



set (src
  include/vtek/vtek_command_pool.h
  include/vtek/vtek_device.h
  include/vtek/vtek_instance.h
  include/vtek/vtek_logging.h
  include/vtek/vtek_main.h
  include/vtek/vtek_physical_device.h
  include/vtek/vtek_queue.h
  include/vtek/vtek_swapchain.h
  include/vtek/vtek_vulkan_helpers.h

  src/impl/vtek_host_allocator.h
  src/impl/vtek_queue_struct.h

  src/vtek_command_pool.cpp
  src/vtek_device.cpp
  src/vtek_host_allocator.cpp
  src/vtek_instance.cpp
  src/vtek_logging.cpp
  src/vtek_main.cpp
  src/vtek_physical_device.cpp
  src/vtek_queue.cpp
  src/vtek_swapchain.cpp
  src/vtek_vulkan_helpers.cpp
  )

if(VTEK_SHARED_LIB)
  add_library(${PROJECT_NAME} SHARED ${src})
else()
  add_library(${PROJECT_NAME} ${src})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/include/vtek)
target_include_directories(${PROJECT_NAME} PRIVATE external/spdlog/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

if(VTEK_BUILD_WARNINGS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
	target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra) # -Werror
  endif()
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC spdlog::spdlog)
