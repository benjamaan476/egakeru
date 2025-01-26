project(renderer_vulkan)

include_directories(${engine_SOURCE_DIR})

file(GLOB_RECURSE SOURCES
    ${engine_SOURCE_DIR}/*.cpp
	*.cpp	
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
)

target_link_system_libraries(
  renderer_vulkan
  PRIVATE
          spdlog::spdlog  
          glfw
)
