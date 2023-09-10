
file(GLOB SOURCES
    log/*.h
    log/*.cpp
)
add_executable(engine main.cpp ${SOURCES})

target_link_libraries(
  engine
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings)

target_link_system_libraries(
  engine
  PRIVATE
          spdlog::spdlog
)

target_include_directories(engine PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
