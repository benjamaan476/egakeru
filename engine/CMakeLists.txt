project(engine)
include_directories(${engine_SOURCE_DIR})


file(GLOB SOURCES
    log/*.cpp
    platform/*.cpp
    platform/windows/*.cpp
)

set(HEADERS 
    log/log.h
    platform/platform.h)

add_library(engine SHARED ${SOURCES})

target_link_libraries(
  engine
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings
)

target_link_system_libraries(
  engine
  PRIVATE
          spdlog::spdlog     
          ${Vulkan_LIBRARIES}
          glfw

)