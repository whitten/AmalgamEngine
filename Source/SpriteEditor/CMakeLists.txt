cmake_minimum_required(VERSION 3.16)

message(STATUS "Configuring Amalgam Engine SpriteEditor")

add_executable(SpriteEditor "")

target_include_directories(SpriteEditor
    PRIVATE
        ${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} 
        ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_GFX_INCLUDE_DIRS} ${SDL2PP_INCLUDE_DIRS}
)

# Inherit Shared's precompiled header.
target_precompile_headers(SpriteEditor REUSE_FROM SharedLib)

target_link_libraries(SpriteEditor
    PRIVATE
        ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES} ${SDL2_TTF_LIBRARIES}
        ${SDL2_GFX_LIBRARIES} ${SDL2PP_LIBRARIES}
        SDL2pp
        AmalgamUI
        EnTT::EnTT
        nfd
        nlohmann_json::nlohmann_json
        SharedLib
)

# Compile with C++20.
target_compile_features(SpriteEditor PRIVATE cxx_std_20)
set_target_properties(SpriteEditor PROPERTIES CXX_EXTENSIONS OFF)

# Enable compile warnings.
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(SpriteEditor PUBLIC -Wall -Wextra)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(SpriteEditor PUBLIC /W3 /permissive-)
endif()

# Copy assets to the build directory.
file(COPY ${PROJECT_SOURCE_DIR}/Resources/SpriteEditor/Common/Assets/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Assets/)

# Build all of the subdirectories
add_subdirectory(Config)
add_subdirectory(DataModel)
add_subdirectory(Launch)
add_subdirectory(Renderer)
add_subdirectory(UserInterface)
