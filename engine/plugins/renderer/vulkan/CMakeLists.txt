project(renderer_vulkan)

include_directories(${engine_SOURCE_DIR})

file(GLOB_RECURSE SOURCES
  command_buffer.cpp
  fence.cpp
  pipeline.cpp
  renderer_vulkan.cpp
  swapchain.cpp
  vulkan_geometry.cpp
  vulkan_render_target.cpp
  vulkan_renderbuffer.cpp
  vulkan_renderpass.cpp
  vulkan_shader.cpp
  vulkan_texture.cpp

    ${engine_SOURCE_DIR}/event.cpp
    ${engine_SOURCE_DIR}/identifier.cpp
    ${engine_SOURCE_DIR}/keymap.cpp
    ${engine_SOURCE_DIR}/ray.cpp
    ${engine_SOURCE_DIR}/application/application.cpp
    ${engine_SOURCE_DIR}/containers/ring_queue.cpp
    ${engine_SOURCE_DIR}/debug/debug_box3d.cpp
    ${engine_SOURCE_DIR}/debug/debug_console.cpp
    ${engine_SOURCE_DIR}/debug/debug_frustum.cpp
    ${engine_SOURCE_DIR}/debug/debug_grid.cpp
    ${engine_SOURCE_DIR}/debug/debug_line.cpp
    ${engine_SOURCE_DIR}/engine/engine.cpp

    ${engine_SOURCE_DIR}/log/log.cpp

    ${engine_SOURCE_DIR}/loaders/image_loader.cpp
    ${engine_SOURCE_DIR}/loaders/resource_loader.cpp
    ${engine_SOURCE_DIR}/loaders/material_loader.cpp
    ${engine_SOURCE_DIR}/loaders/binary_loader.cpp
    ${engine_SOURCE_DIR}/loaders/shader_loader.cpp
    ${engine_SOURCE_DIR}/loaders/text_loader.cpp
    ${engine_SOURCE_DIR}/loaders/mesh_loader.cpp
    ${engine_SOURCE_DIR}/loaders/bitmap_font_loader.cpp
    ${engine_SOURCE_DIR}/loaders/system_font_loader.cpp
    ${engine_SOURCE_DIR}/loaders/scene_loader.cpp
    ${engine_SOURCE_DIR}/plugins/audio/audio_loader.cpp
    ${engine_SOURCE_DIR}/plugins/audio/oal_plugin.cpp

    ${engine_SOURCE_DIR}/platform/platform.cpp
    ${engine_SOURCE_DIR}/platform/filesystem.cpp

    ${engine_SOURCE_DIR}/renderer/camera.cpp
    ${engine_SOURCE_DIR}/renderer/render_graph.cpp
    ${engine_SOURCE_DIR}/renderer/render_target.cpp
    ${engine_SOURCE_DIR}/renderer/renderbuffer.cpp
    ${engine_SOURCE_DIR}/renderer/renderer_frontend.cpp
    ${engine_SOURCE_DIR}/renderer/renderer_types.cpp
    ${engine_SOURCE_DIR}/renderer/renderpass.cpp
    ${engine_SOURCE_DIR}/renderer/viewport.cpp

    ${engine_SOURCE_DIR}/resources/geometry.cpp
    ${engine_SOURCE_DIR}/resources/job.cpp
    ${engine_SOURCE_DIR}/resources/material.cpp
    ${engine_SOURCE_DIR}/resources/mesh.cpp
    ${engine_SOURCE_DIR}/resources/resource.cpp
    ${engine_SOURCE_DIR}/resources/shader.cpp
    ${engine_SOURCE_DIR}/resources/skybox.cpp
    ${engine_SOURCE_DIR}/resources/texture.cpp
    ${engine_SOURCE_DIR}/resources/transform.cpp
    ${engine_SOURCE_DIR}/resources/ui_text.cpp

    ${engine_SOURCE_DIR}/systems/audio_system.cpp
    ${engine_SOURCE_DIR}/systems/camera_system.cpp
    ${engine_SOURCE_DIR}/systems/console_system.cpp
    ${engine_SOURCE_DIR}/systems/evar_system.cpp
    ${engine_SOURCE_DIR}/systems/font_system.cpp
    ${engine_SOURCE_DIR}/systems/geometry_system.cpp
    ${engine_SOURCE_DIR}/systems/input.cpp
    ${engine_SOURCE_DIR}/systems/job_system.cpp
    ${engine_SOURCE_DIR}/systems/light_system.cpp
    ${engine_SOURCE_DIR}/systems/material_system.cpp
    ${engine_SOURCE_DIR}/systems/resource_system.cpp
    ${engine_SOURCE_DIR}/systems/shader_system.cpp
    ${engine_SOURCE_DIR}/systems/system.cpp
    ${engine_SOURCE_DIR}/systems/texture_system.cpp
)

add_library(renderer_vulkan SHARED ${SOURCES})

if (WIN32)
    target_sources(renderer_vulkan PRIVATE
    ${engine_SOURCE_DIR}/platform/windows/platform_windows.cpp
    )
endif()
if (LINUX)
    target_sources(renderer_vulkan PRIVATE
    ${engine_SOURCE_DIR}/platform/linux/platform_linux.cpp
    )
endif()

target_link_libraries(
  renderer_vulkan
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings
          Vulkan::Vulkan
          Vulkan::Headers
          OpenAL::OpenAL
          glm::glm
)

target_link_system_libraries(
  renderer_vulkan
  PRIVATE
          spdlog::spdlog  
          glfw
)
