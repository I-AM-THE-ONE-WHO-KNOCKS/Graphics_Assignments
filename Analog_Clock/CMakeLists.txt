# CMake entry point
cmake_minimum_required (VERSION 3.0)
project (Analog_Clock)

find_package(OpenGL REQUIRED)

if( CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR )
    message( FATAL_ERROR "Please select another Build Directory ! (and give it a clever name, like bin_Visual2012_64bits/)" )
endif()
if( CMAKE_SOURCE_DIR MATCHES " " )
    message( "Your Source Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()
if( CMAKE_BINARY_DIR MATCHES " " )
    message( "Your Build Directory contains spaces. If you experience problems when compiling, this can be the cause." )
endif()

# Compile external dependencies
add_subdirectory (Libs)

# On Visual 2005 and above, this module can set the debug working directory
cmake_policy(SET CMP0026 OLD)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/Libs/rpavlik-cmake-modules-fe2273")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

include_directories(
    Libs/glfw-3.1.2/include/
    Libs/glm-0.9.7.1/
    Libs/glew-1.13.0/include/
    common/include/
)

set(ALL_LIBS
    ${OPENGL_LIBRARY}
    glfw
    GLEW_1130
    glut
)

add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)

# Solar_System_Clock
add_executable(Solar_System_Clock
    Solar_System_Clock/Solar_System_Clock.cpp
    common/Sphere.cpp

    Solar_System_Clock/shader_solar.frag
    Solar_System_Clock/shader_solar.vert
)
target_link_libraries(Solar_System_Clock
    ${ALL_LIBS}
)
# Xcode and Visual working directories
set_target_properties(Solar_System_Clock PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}Solar_System_Clock/")
create_target_launcher(Solar_System_Clock WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Solar_System_Clock/")
