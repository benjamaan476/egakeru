# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(egakeru_setup_dependencies)

  # For each dependency, see if it's
  # already been provided to us by a parent project

  if(NOT TARGET spdlog::spdlog)
    find_package(spdlog REQUIRED)
  endif()

  if(NOT TARGET Vulkan_LIBRARIES)
     find_package(Vulkan REQUIRED COMPONENTS glslc)
     include_directories(${Vulkan_INCLUDE_DIRS})
  endif()

  if(NOT TARGET glfw)
        find_package(glfw3 CONFIG REQUIRED)
        include_directories(${GLFW_INCLUDE_DIRS})
  endif()

endfunction()
