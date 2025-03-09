cmake_minimum_required(VERSION 3.11)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set (CMAKE_C_STANDARD 11)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=c11 -O0")

project(mcEliece)

# Config options
option(BUILD_RAYGUI_EXAMPLES "Build the examples." OFF)

# Force building examples if building in the root as standalone.
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(BUILD_RAYGUI_EXAMPLES TRUE)
endif()

# Directory Variables
set(RAYGUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/raygui-src)
set(RAYGUI_SRC ${RAYGUI_ROOT}/src)
set(RAYGUI_EXAMPLES ${RAYGUI_ROOT}/examples)

# raygui
add_library(raygui INTERFACE)

set(RAYLIB_VERSION 5.0)
set(RAYGUI_VERSION 4.0)

find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
  endif()
endif()

find_package(raygui ${RAYGUI_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raygui_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raygui
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raygui/archive/refs/tags/${RAYGUI_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raygui)
  if (NOT raygui_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_MakeAvailable(raygui)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
  endif()
endif()

file(GLOB sources ${RAYGUI_SRC}/*.h)
set(RAYGUI_HEADERS ${sources})
install(FILES
    ${RAYGUI_HEADERS} DESTINATION include
)
target_include_directories(raygui INTERFACE ${RAYGUI_SRC})

# Examples
if(${BUILD_RAYGUI_EXAMPLES})
    find_package(raylib ${RAYLIB_VERSION} QUIET)

    # Get the sources together
    set(example_dirs
        controls_test_suite
        custom_file_dialog
        custom_sliders
        image_exporter
        image_importer_raw
        portable_window
        property_list
        scroll_panel
        style_selector
    )

    set(example_sources)
    set(example_resources)

    foreach(example_dir ${example_dirs})
        # Get the .c files
        file(GLOB sources ${RAYGUI_EXAMPLES}/${example_dir}/*.c)
        list(APPEND example_sources ${sources})

        # Any any resources
        file(GLOB resources ${RAYGUI_EXAMPLES}/${example_dir}/resources/*)
        list(APPEND example_resources ${resources})
    endforeach()

    # Do each example
    foreach(example_source ${example_sources})
        # Create the basename for the example
        get_filename_component(example_name ${example_source} NAME)
        string(REPLACE ".c" "${OUTPUT_EXT}" example_name ${example_name})

        # Setup the example
        add_executable(${example_name} ${example_source})

        target_link_libraries(${example_name} PUBLIC raylib raygui)

        string(REGEX MATCH ".*/.*/" resources_dir ${example_source})
        string(APPEND resources_dir "resources")
    endforeach()

    # Copy all of the resource files to the destination
    file(COPY ${example_resources} DESTINATION "resources/")
endif()

if(${BUILD_RAYGUI_EXAMPLES})

  add_subdirectory(libOQS)

  add_executable(${PROJECT_NAME} main.c config_files.c config_files.h)
  # target_include_directories(LibOQS INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/LibOQS)
  target_link_libraries(${PROJECT_NAME} PUBLIC raylib raygui libOQS)
endif()