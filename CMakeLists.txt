cmake_minimum_required(VERSION 3.16)

project(cplchess CXX)

# Set the default build the to Debug if it's not set on the command line.
# This is not done for multi configuration generator like Visual Studio
# (detected thought CMAKE_CONFIGURATION_TYPES).
if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)

    # Set the possible values of build type for cmake-gui/ccmake
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else ()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif ()

add_library(cplchess_lib OBJECT
    Square.cpp
    Move.cpp
    Piece.cpp
    Board.cpp
    CastlingRights.cpp
    Fen.cpp
    PrincipalVariation.cpp
    Engine.cpp
    EngineFactory.cpp
    Uci.cpp
)

target_include_directories(cplchess_lib PUBLIC .)

add_executable(cplchess Main.cpp)
target_link_libraries(cplchess cplchess_lib)

include(CTest)
add_subdirectory(Tests/)
