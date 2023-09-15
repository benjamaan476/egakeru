
file(GLOB SOURCES
    log/*.h
    platform/*.h
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

set_target_properties(sandbox PROPERTIES LINK_DEPENDS $<TARGET_FILE:engine>)

add_custom_command(
    TARGET sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:sandbox> $<TARGET_FILE_DIR:sandbox>
    COMMAND_EXPAND_LISTS
  )