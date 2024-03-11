cmake_minimum_required(VERSION 3.26)
project(AudioPlay)

set(CMAKE_CXX_STANDARD 23)

add_executable(AudioPlay main.cpp OpenSans_Regular_ttf.h OpenSans_Regular_ttf.cpp IWINDOW.cpp IWINDOW.h
        import.h
        MusicBlock.h ForWindows/tinyfiledialogs.h ForWindows/tinyfiledialogs.c
        QueueSystem.h
        UUID.cpp
        UUID.h
        ForWindows/Windows.h
        ForWindows/Windows.cpp
        DeletionQueue.h
        PlayLists.h
        JSONEditor.h
)

include(ImGui.cmake)
add_imgui_and_sdl3(AudioPlay)
target_link_libraries(AudioPlay PUBLIC SDL3_mixer::SDL3_mixer)

configure_file(${CMAKE_CURRENT_LIST_DIR}/Assets/Icon.ico ${CMAKE_CURRENT_BINARY_DIR}/Assets/Icon.ico COPYONLY)