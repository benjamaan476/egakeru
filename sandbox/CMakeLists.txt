
file(GLOB SOURCES
    entry.cpp
    sandbox_application.cpp
    sandbox_keybinds.cpp
)

add_executable(sandbox ${SOURCES})

include_directories(${engine_SOURCE_DIR})

target_link_libraries(
  sandbox
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings  
          Vulkan::Vulkan
          Vulkan::Headers
          OpenAL::OpenAL
          $<TARGET_OBJECTS:renderer_vulkan>
          $<TARGET_OBJECTS:engine>)

target_link_system_libraries(
  sandbox
  PRIVATE
          spdlog::spdlog
          glfw
          glm::glm
)
target_include_directories(sandbox PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")

set_target_properties(sandbox PROPERTIES LINK_DEPENDS $<TARGET_FILE:engine>)

# add_custom_command(
#     TARGET sandbox POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:sandbox> $<TARGET_FILE_DIR:sandbox>
#     COMMAND_EXPAND_LISTS
#   )
