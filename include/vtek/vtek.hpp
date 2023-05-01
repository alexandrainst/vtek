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

#include "vtek_main.hpp"
#include "vtek_types.hpp"

#include "vtek_application_window.hpp"
#include "vtek_command_buffer.hpp"
#include "vtek_command_pool.hpp"
#include "vtek_device.hpp"
#include "vtek_graphics_pipeline.hpp"
#include "vtek_input.hpp"
#include "vtek_instance.hpp"
#include "vtek_logging.hpp"
#include "vtek_queue.hpp"
#include "vtek_render_pass.hpp"
#include "vtek_physical_device.hpp"
#include "vtek_shaders.hpp"
#include "vtek_submit_info.hpp"
#include "vtek_swapchain.hpp"
