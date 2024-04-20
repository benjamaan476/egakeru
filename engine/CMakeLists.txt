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
          glm::glm
)

target_link_libraries(engine PRIVATE ${OPENAL_LIBRARY})

target_link_system_libraries(
  engine
  PRIVATE
          spdlog::spdlog     
          glfw
)


