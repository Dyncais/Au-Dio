set(imgui_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/imgui)
include(FetchContent)
FetchContent_Populate(imgui
        URL https://github.com/ocornut/imgui/archive/docking.zip
        GIT_TAG 36ffdd69d304659388ad3d3be487a467de65eaf6
        SOURCE_DIR ${imgui_DIR}
)
add_library(imgui STATIC
        ${imgui_DIR}/imgui.cpp
        ${imgui_DIR}/imgui_demo.cpp
        ${imgui_DIR}/imgui_draw.cpp
        ${imgui_DIR}/imgui_widgets.cpp
        ${imgui_DIR}/imgui_tables.cpp
        ${imgui_DIR}/backends/imgui_impl_sdl3.cpp
        ${imgui_DIR}/backends/imgui_impl_sdlrenderer3.cpp
        ${imgui_DIR}/misc/cpp/imgui_stdlib.cpp
        )
target_include_directories(imgui PUBLIC ${imgui_DIR}
        PUBLIC ${imgui_DIR}
)

set(SDL_SHARED FALSE CACHE BOOL "Build a SDL shared library (if available)")
set(SDL_STATIC TRUE CACHE BOOL "Build a SDL static library (if available)")
FetchContent_Declare(
        SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG 983544a53e42ed136d1518c62402dad4c2f1958a
        GIT_SHALLOW FALSE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL)
set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl-src" PROPERTY EXCLUDE_FROM_ALL TRUE)

target_link_libraries(imgui PRIVATE SDL3::SDL3)

function(add_imgui_and_sdl3 target)
    target_link_libraries(${target} PUBLIC imgui PUBLIC SDL3::SDL3)
endfunction()

set(BUILD_SHARED_LIBS  OFF)

FetchContent_Declare(
        SDL_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG 43bfc20637e2e5274f3f88c187778e43bcfbb684
        GIT_SHALLOW FALSE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(SDL_mixer)
set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/sdl_mixer-src" PROPERTY EXCLUDE_FROM_ALL TRUE)