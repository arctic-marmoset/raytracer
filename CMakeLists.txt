cmake_minimum_required(VERSION 3.14 FATAL_ERROR)
project(raytracer)

set(CMAKE_CXX_STANDARD 23)

add_executable(raytracer
    src/raytracer.cpp
)

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

target_link_libraries(raytracer
    PRIVATE
        fmt::fmt-header-only
        glm::glm
        ZLIB::ZLIB
)

find_package(Doxygen)

if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation using Doxygen"
        VERBATIM
    )
endif()
