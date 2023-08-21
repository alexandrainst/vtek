/****************************************************************
 * vtek
 * A C++ Vulkan framework for offloading work to the GPU
 *---------------------------------------------------------------
 * Copyright (c) 2023 Alexandra Instituttet
 * Designed and implemented by Alexander Christensen
 * <alexander.christensen@alexandra.dk>
 *
 * All rights reserved.
 ****************************************************************/

#pragma once

/*
 * Main header for the vtek Vulkan framework.
 * Include this file in your project to get access to
 * all of vtek's functionality.
 */

#include "vtek_glm_includes.hpp"

#include "vtek_colors.hpp"
#include "vtek_main.hpp"
#include "vtek_object_handles.hpp"
#include "vtek_types.hpp"
#include "vtek_vulkan_version.hpp"

#include "vtek_application_window.hpp"
#include "vtek_allocator.hpp"
#include "vtek_buffer.hpp"
#include "vtek_camera.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_command_pool.hpp"
#include "vtek_commands.hpp"
#include "vtek_descriptor_pool.hpp"
#include "vtek_descriptor_set.hpp"
#include "vtek_descriptor_set_layout.hpp"
#include "vtek_device.hpp"
#include "vtek_graphics_pipeline.hpp"
#include "vtek_image.hpp"
#include "vtek_input.hpp"
#include "vtek_instance.hpp"
#include "vtek_logging.hpp"
#include "vtek_models.hpp"
#include "vtek_queue.hpp"
#include "vtek_render_pass.hpp"
#include "vtek_physical_device.hpp"
#include "vtek_push_constants.hpp"
#include "vtek_sampler.hpp"
#include "vtek_shaders.hpp"
#include "vtek_submit_info.hpp"
#include "vtek_swapchain.hpp"
#include "vtek_uniform_data.hpp"
#include "vtek_vertex_data.hpp"
