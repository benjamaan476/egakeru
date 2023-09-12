
file(GLOB SOURCES
    log/*.h
)

add_executable(sandbox main.cpp ${SOURCES})

include_directories(${engine_SOURCE_DIR})

target_link_libraries(
  sandbox
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings
          engine)

target_link_system_libraries(
  sandbox
  PRIVATE
          spdlog::spdlog
)

target_include_directories(sandbox PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

add_custom_command(TARGET sandbox POST_BUILD        # Adds a post-build event to MyTest
    COMMAND ${CMAKE_COMMAND} -E copy_if_different  # which executes "cmake - E copy_if_different..."
        "${engine_BINARY_DIR}/engine.dll"      # <--this is in-file
        $<TARGET_FILE_DIR:sandbox>)                 # <--this is out-file path
