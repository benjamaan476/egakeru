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
)

add_library(renderer_vulkan SHARED ${SOURCES})

target_link_libraries(
  renderer_vulkan
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings
          Vulkan::Vulkan
          Vulkan::Headers
          OpenAL::OpenAL
          glm::glm
          $<TARGET_OBJECTS:engine>
)

target_link_system_libraries(
  renderer_vulkan
  PRIVATE
          spdlog::spdlog  
          glfw
)
