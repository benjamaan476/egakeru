project(engine)

add_subdirectory(plugins)

include_directories(${engine_SOURCE_DIR})


set(SOURCES
    ${engine_SOURCE_DIR}/editor_gizmo.cpp
    ${engine_SOURCE_DIR}/event.cpp
    ${engine_SOURCE_DIR}/identifier.cpp
    ${engine_SOURCE_DIR}/keymap.cpp
    ${engine_SOURCE_DIR}/ray.cpp
    application/application.cpp
    containers/ring_queue.cpp
    debug/debug_box3d.cpp
    debug/debug_console.cpp
    debug/debug_frustum.cpp
    debug/debug_grid.cpp
    debug/debug_line.cpp
    engine/engine.cpp
    loaders/binary_loader.cpp
    loaders/bitmap_font_loader.cpp
    loaders/image_loader.cpp
    loaders/material_loader.cpp
    loaders/mesh_loader.cpp
    loaders/resource_loader.cpp
    loaders/scene_loader.cpp
    loaders/shader_loader.cpp
    loaders/system_font_loader.cpp
    loaders/text_loader.cpp
    log/log.cpp
    platform/platform.cpp
    platform/filesystem.cpp
    renderer/camera.cpp
    renderer/render_graph.cpp
    renderer/render_target.cpp
    renderer/renderbuffer.cpp
    renderer/renderer_frontend.cpp
    renderer/renderer_types.cpp
    renderer/renderpass.cpp
    renderer/viewport.cpp
    renderer/passes/editor_pass.cpp
    renderer/passes/scene_pass.cpp
    renderer/passes/skybox_pass.cpp
    renderer/passes/ui_pass.cpp
    resources/geometry.cpp
    resources/job.cpp
    resources/material.cpp
    resources/mesh.cpp
    resources/resource.cpp
    resources/shader.cpp
    resources/skybox.cpp
    resources/texture.cpp
    resources/transform.cpp
    resources/ui_text.cpp
    scenes/simple_scene.cpp
    systems/audio_system.cpp
    systems/camera_system.cpp
    systems/console_system.cpp
    systems/evar_system.cpp
    systems/font_system.cpp
    systems/geometry_system.cpp
    systems/input.cpp
    systems/job_system.cpp
    systems/light_system.cpp
    systems/material_system.cpp
    systems/resource_system.cpp
    systems/shader_system.cpp
    systems/system.cpp
    systems/texture_system.cpp

    plugins/audio/audio_loader.cpp
    plugins/audio/oal_plugin.cpp
)

set(HEADERS 
    log/log.h
    platform/platform.h
    engine/*.h
    application/*.h
    event.h
    input.h
    renderer/*.h
    renderer/vulkan/*.h
    resources/*.h
    systems/*.h
    loaders/*.h
    contianers/*.h
    scenes/*.h
    interfaces/*.h
    renderer/passes/*.h
)

add_library(engine SHARED ${SOURCES})
if (WIN32)
    target_sources(engine PRIVATE
    platform/windows/platform_windows.cpp
    )
endif()
if (LINUX)
    target_sources(engine PRIVATE
    platform/linux/platform_linux.cpp
    )
endif()

if(DEFINED TRACY_ENABLE)
add_library(engine PRIVATE  ${engine_SOURCE_DIR}/../tracy/public/TracyClient.cpp)
endif()

if(DEFINED TRACY_ENABLE)
    message("Enabling tracy profiling")
    target_compile_definitions(engine PRIVATE TRACY_ENABLE)
endif()

if(DEFINED TRACY_ON_DEMAND)
    message("Enabling tracy on demand profiling")
    target_compile_definitions(engine PRIVATE TRACY_ON_DEMAND)
endif()

if(DEFINED TRACY_MEMORY)
    target_compile_definitions(engine PRIVATE TRACY_MEMORY)
endif()

target_link_libraries(
  engine
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings
          OpenAL::OpenAL
          glm::glm
)

target_link_system_libraries(
  engine
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
