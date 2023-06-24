add_compile_definitions(_PROJECT_DIR_="${CMAKE_CURRENT_SOURCE_DIR}")

set(RENDERER_SOURCE
    renderer/Buffer.cpp
    renderer/CommandBuffer.cpp
    renderer/Initialisers.cpp
    renderer/Pipeline.cpp
    renderer/Renderer.cpp
    renderer/RendererCore.cpp
    renderer/Sprite.cpp
    renderer/SpriteRenderer.cpp
    renderer/Texture.cpp)

set(WINDOW_SOURCE
    Platform/Window.cpp
    Platform/Windows/WindowsWindow.cpp)

set(SOURCE
    Application.cpp
    ApplicationCore.cpp
    Instrumentor.cpp
    ui/gui.cpp
    Log/Log.cpp
    ${RENDERER_SOURCE}
    ${WINDOW_SOURCE}
    )

add_executable(engine main.cpp ${SOURCE})

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


target_include_directories(engine PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include" PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
