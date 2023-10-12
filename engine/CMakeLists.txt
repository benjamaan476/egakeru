project(engine)
include_directories(${engine_SOURCE_DIR})


file(GLOB SOURCES
    log/*.cpp
    platform/*.cpp
    platform/windows/*.cpp
    application/*.cpp
    game/*.cpp
    event.cpp
    input.cpp
    renderer/*.cpp
    renderer/vulkan/*.cpp
    resources/*.cpp
    systems/*.cpp
    loaders/*.cpp
)

set(HEADERS 
    log/log.h
    platform/platform.h
    application/*.h
    game/*.h
    event.h
    input.h
    renderer/*.h
    renderer/vulkan/*.h
    resources/*.h
    systems/*.h
    loaders/*.h
)

add_library(engine SHARED ${SOURCES})

target_link_libraries(
  engine
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings
          Vulkan::Vulkan
          Vulkan::Headers
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

add_custom_command(
    COMMAND
        ${CMAKE_COMMAND} -E make_directory ${SHADER_BINARY_DIR}
    OUTPUT ${SHADER_BINARY_DIR}
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