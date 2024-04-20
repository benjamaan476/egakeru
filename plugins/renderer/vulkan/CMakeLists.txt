project(renderer_vulkan)

file(GLOB_RECURSE SOURCES
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_library(renderer_vulkan SHARED ${SOURCES})

target_include_directories(renderer_vulkan PRIVATE ${engine_SOURCE_DIR})
target_link_libraries(
  renderer_vulkan
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings
          Vulkan::Vulkan
          Vulkan::Headers
          glm::glm
          engine
)
target_link_system_libraries(
  renderer_vulkan
  PRIVATE
          spdlog::spdlog     
          glfw
)

set(SHADER_SOURCE_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)
set(SHADER_BINARY_DIR ${CMAKE_SOURCE_DIR}/assets/shaders)

file(GLOB SHADERS
    ${SHADER_SOURCE_DIR}/*.frag
    ${SHADER_SOURCE_DIR}/*.vert
)

foreach(source IN LISTS SHADERS)
    get_filename_component(FILENAME ${source} NAME)
    add_custom_command(
            COMMAND
                ${glslc_executable} ${source}
                # -MD -MF
                -o ${SHADER_BINARY_DIR}/${FILENAME}.spv
            OUTPUT ${SHADER_BINARY_DIR}/${FILENAME}.spv
            DEPENDS ${source} ${SHADER_BINARY_DIR}
            COMMENT "Compiling ${FILENAME}"
    )
    list(APPEND SPV_HEADERS ${SHADER_BINARY_DIR}/${FILENAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_HEADERS})