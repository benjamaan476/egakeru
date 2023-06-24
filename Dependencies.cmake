# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(egakeru_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT TARGET glm::glm)
    find_package(glm CONFIG REQUIRED)
    include_directories(${GLM_INCLUDE_DIRS})
  endif()

  if(NOT TARGET imgui::imgui)
    find_package(imgui CONFIG REQUIRED)
    include_directories(${Imgui_INCLUDE_DIRS})
  endif()

  if(NOT TARGET spdlog::spdlog)
    find_package(spdlog REQUIRED)
  endif()

  if(NOT TARGET Catch2::Catch2)
    find_package(Catch2 CONFIG REQUIRED)
  endif()


endfunction()
