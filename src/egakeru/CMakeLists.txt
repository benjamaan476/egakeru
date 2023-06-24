add_executable(engine main.cpp)

target_link_libraries(
  engine
  PRIVATE egakeru::egakeru_options
          egakeru::egakeru_warnings
          glm::glm
          glfw
          ${Vulkan_LIBRARIES}
          spdlog::spdlog
          imgui::imgui)

#target_link_system_libraries(
#  engine
#  PRIVATE
#          CLI11::CLI11
#          fmt::fmt
#          spdlog::spdlog
#          lefticus::tools
#          ftxui::screen
#          ftxui::dom
#          ftxui::component)

target_include_directories(engine PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
