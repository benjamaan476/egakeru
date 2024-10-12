cmake_minimum_required(VERSION 3.21)

# This template attempts to be "fetch_content"-able
# so that it works well with tools like CPM or other
# manual dependency management

# Only set the cxx_standard if it is not set by someone else
  set(CMAKE_CXX_STANDARD 23)

# strongly encouraged to enable this globally to avoid conflicts between
# -Wpedantic being enabled and -std=c++20 and -std=gnu++20 for example
# when compiling with PCH enabled
set(CMAKE_CXX_EXTENSIONS OFF)

# Set the project name and language
project(
  egakeru
  VERSION 0.0.1
  DESCRIPTION ""
  HOMEPAGE_URL "https://github.com/benjamaan476/egakeru"
  LANGUAGES CXX C)

include(cmake/PreventInSourceBuilds.cmake)
include(ProjectOptions.cmake)


egakeru_setup_options()

egakeru_global_options()
include(Dependencies.cmake)
egakeru_setup_dependencies()

egakeru_local_options()

# don't know if this should be set globally from here or not...
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

#set(GIT_SHA
#    "Unknown"
#    CACHE STRING "SHA this build was generated from")
#string(
#  SUBSTRING "${GIT_SHA}"
#            0
#            8
#            GIT_SHORT_SHA)

target_compile_features(egakeru_options INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

add_library(egakeru::egakeru_options ALIAS egakeru_options)
add_library(egakeru::egakeru_warnings ALIAS egakeru_warnings)

#option(TRACY_ENABLE ON)
#option(TRACY_ON_DEMAND "" ON)
#option(TRACY_MEMORY ON)

if(DEFINED TRACY_ENABLE)
add_subdirectory (tracy)
endif()

add_definitions(-DENABLE_DEBUG_MACRO)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Adding the src:
add_subdirectory(engine)
add_subdirectory(sandbox)

# Don't even look at tests if we're not top level
if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

# Adding the tests:
#include(CTest)

#if(BUILD_TESTING)
#  message(AUTHOR_WARNING "Building Tests. Be sure to check out test/constexpr_tests.cpp for constexpr testing")
#  add_subdirectory(test)
#endif()


# If MSVC is being used, and ASAN is enabled, we need to set the debugger environment
# so that it behaves well with MSVC's debugger, and we can run the target from visual studio
if(MSVC)
  get_all_installable_targets(all_targets)
  message("all_targets=${all_targets}")
  set_target_properties(${all_targets} PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
endif()

# set the startup project for the "play" button in MSVC
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT engine)

if(CMAKE_SKIP_INSTALL_RULES)
  return()
endif()

include(cmake/PackageProject.cmake)

# Add other targets that you want installed here, by default we just package the one executable
# we know we want to ship
egakeru_package_project(
  TARGETS
  engine
  egakeru_options
  egakeru_warnings
  # FIXME: this does not work! CK
  # PRIVATE_DEPENDENCIES_CONFIGURED project_options project_warnings
)

# Experience shows that explicit package naming can help make it easier to sort
# out potential ABI related issues before they start, while helping you
# track a build to a specific GIT SHA
set(CPACK_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-${CMAKE_PROJECT_VERSION}-${GIT_SHORT_SHA}-${CMAKE_SYSTEM_NAME}-${CMAKE_BUILD_TYPE}-${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}"
)

include(CPack)
