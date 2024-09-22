include(cmake/SystemLink.cmake)
include(CMakeDependentOption)
include(CheckCXXCompilerFlag)


macro(egakeru_supports_sanitizers)
  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND NOT WIN32)
    set(SUPPORTS_UBSAN ON)
  else()
    set(SUPPORTS_UBSAN OFF)
  endif()

  if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES ".*GNU.*") AND WIN32)
    set(SUPPORTS_ASAN OFF)
  else()
    set(SUPPORTS_ASAN ON)
  endif()

  set(SUPPORTS_ASAN OFF)
  set(SUPPORTS_UBSAN OFF)
endmacro()

macro(egakeru_setup_options)
  option(egakeru_ENABLE_HARDENING "Enable hardening" ON)
  option(egakeru_ENABLE_COVERAGE "Enable coverage reporting" OFF)
  cmake_dependent_option(
    egakeru_ENABLE_GLOBAL_HARDENING
    "Attempt to push hardening options to built dependencies"
    ON
    egakeru_ENABLE_HARDENING
    OFF)

  egakeru_supports_sanitizers()

  if(NOT PROJECT_IS_TOP_LEVEL OR egakeru_PACKAGING_MAINTAINER_MODE)
    option(egakeru_ENABLE_IPO "Enable IPO/LTO" OFF)
    option(egakeru_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
    option(egakeru_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(egakeru_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
    option(egakeru_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(egakeru_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
    option(egakeru_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(egakeru_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(egakeru_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(egakeru_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(egakeru_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
    option(egakeru_ENABLE_PCH "Enable precompiled headers" OFF)
    option(egakeru_ENABLE_CACHE "Enable ccache" OFF)
  else()
    option(egakeru_ENABLE_IPO "Enable IPO/LTO" ON)
    option(egakeru_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
    option(egakeru_ENABLE_USER_LINKER "Enable user-selected linker" OFF)
    option(egakeru_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" ${SUPPORTS_ASAN})
    option(egakeru_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
    option(egakeru_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" ${SUPPORTS_UBSAN})
    option(egakeru_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
    option(egakeru_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
    option(egakeru_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
    option(egakeru_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
    option(egakeru_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
    option(egakeru_ENABLE_PCH "Enable precompiled headers" OFF)
    option(egakeru_ENABLE_CACHE "Enable ccache" OFF)
  endif()

  if(NOT PROJECT_IS_TOP_LEVEL)
    mark_as_advanced(
      egakeru_ENABLE_IPO
      egakeru_WARNINGS_AS_ERRORS
      egakeru_ENABLE_USER_LINKER
      egakeru_ENABLE_SANITIZER_ADDRESS
      egakeru_ENABLE_SANITIZER_LEAK
      egakeru_ENABLE_SANITIZER_UNDEFINED
      egakeru_ENABLE_SANITIZER_THREAD
      egakeru_ENABLE_SANITIZER_MEMORY
      egakeru_ENABLE_UNITY_BUILD
      egakeru_ENABLE_CLANG_TIDY
      egakeru_ENABLE_CPPCHECK
      egakeru_ENABLE_COVERAGE
      egakeru_ENABLE_PCH
      egakeru_ENABLE_CACHE)
  endif()

  if(LIBFUZZER_SUPPORTED AND (egakeru_ENABLE_SANITIZER_ADDRESS OR egakeru_ENABLE_SANITIZER_THREAD OR egakeru_ENABLE_SANITIZER_UNDEFINED))
    set(DEFAULT_FUZZER ON)
  else()
    set(DEFAULT_FUZZER OFF)
  endif()

  option(egakeru_BUILD_FUZZ_TESTS "Enable fuzz testing executable" ${DEFAULT_FUZZER})

endmacro()

macro(egakeru_global_options)
  if(egakeru_ENABLE_IPO)
    include(cmake/InterproceduralOptimization.cmake)
    egakeru_enable_ipo()
  endif()

  egakeru_supports_sanitizers()

  if(egakeru_ENABLE_HARDENING AND egakeru_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR egakeru_ENABLE_SANITIZER_UNDEFINED
       OR egakeru_ENABLE_SANITIZER_ADDRESS
       OR egakeru_ENABLE_SANITIZER_THREAD
       OR egakeru_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    message("${egakeru_ENABLE_HARDENING} ${ENABLE_UBSAN_MINIMAL_RUNTIME} ${egakeru_ENABLE_SANITIZER_UNDEFINED}")
    egakeru_enable_hardening(egakeru_options ON ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()
endmacro()

macro(egakeru_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(egakeru_warnings INTERFACE)
  add_library(egakeru_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  egakeru_set_project_warnings(
    egakeru_warnings
    ${egakeru_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  if(egakeru_ENABLE_USER_LINKER)
    include(cmake/Linker.cmake)
    configure_linker(egakeru_options)
  endif()

  include(cmake/Sanitizers.cmake)
  egakeru_enable_sanitizers(
    egakeru_options
    ${egakeru_ENABLE_SANITIZER_ADDRESS}
    ${egakeru_ENABLE_SANITIZER_LEAK}
    ${egakeru_ENABLE_SANITIZER_UNDEFINED}
    ${egakeru_ENABLE_SANITIZER_THREAD}
    ${egakeru_ENABLE_SANITIZER_MEMORY})

  set_target_properties(egakeru_options PROPERTIES UNITY_BUILD ${egakeru_ENABLE_UNITY_BUILD})

  if(egakeru_ENABLE_PCH)
    target_precompile_headers(
      egakeru_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(egakeru_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    egakeru_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(egakeru_ENABLE_CLANG_TIDY)
    egakeru_enable_clang_tidy(egakeru_options ${egakeru_WARNINGS_AS_ERRORS})
  endif()

  if(egakeru_ENABLE_CPPCHECK)
    egakeru_enable_cppcheck(${egakeru_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(egakeru_ENABLE_COVERAGE)
    include(cmake/Tests.cmake)
    egakeru_enable_coverage(egakeru_options)
  endif()

  if(egakeru_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(egakeru_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

  if(egakeru_ENABLE_HARDENING AND NOT egakeru_ENABLE_GLOBAL_HARDENING)
    include(cmake/Hardening.cmake)
    if(NOT SUPPORTS_UBSAN 
       OR egakeru_ENABLE_SANITIZER_UNDEFINED
       OR egakeru_ENABLE_SANITIZER_ADDRESS
       OR egakeru_ENABLE_SANITIZER_THREAD
       OR egakeru_ENABLE_SANITIZER_LEAK)
      set(ENABLE_UBSAN_MINIMAL_RUNTIME FALSE)
    else()
      set(ENABLE_UBSAN_MINIMAL_RUNTIME TRUE)
    endif()
    egakeru_enable_hardening(egakeru_options OFF ${ENABLE_UBSAN_MINIMAL_RUNTIME})
  endif()

endmacro()
