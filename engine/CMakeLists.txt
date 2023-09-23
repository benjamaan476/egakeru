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