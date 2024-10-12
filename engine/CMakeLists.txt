project(engine)
include_directories(${engine_SOURCE_DIR})


file(GLOB_RECURSE SOURCES
${engine_SOURCE_DIR}/*.cpp
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
)

add_library(engine SHARED ${SOURCES})

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
          Vulkan::Vulkan
          Vulkan::Headers
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
