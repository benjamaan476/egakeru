project(engine)


file(GLOB SOURCES
    log/*.cpp
)

set(HEADERS 
    log/log.h)

add_library(engine SHARED ${SOURCES})

target_link_libraries(
  engine
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings)

target_link_system_libraries(
  engine
  PRIVATE
          spdlog::spdlog
)