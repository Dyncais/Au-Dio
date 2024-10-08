cmake_minimum_required(VERSION 3.26)
include(StandardProjectSettings.cmake)
if(WIN32)
        set(VCPKG_TARGET_TRIPLET x64-windows)
endif()
include(vendor/vcpkg/scripts/buildsystems/vcpkg.cmake)
project(AudioPlay)

set(CMAKE_CXX_STANDARD 23)
include(CompilerWarnings.cmake)

add_subdirectory(vendor/libsndfile)
add_executable(AudioPlayTest test.cpp OpenSans_Regular_ttf.h OpenSans_Regular_ttf.cpp IWINDOW.cpp IWINDOW.h)

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
set_project_warnings(AudioPlay)

add_subdirectory(vendor/fftw)
find_package(taglib CONFIG REQUIRED)
include(ImGui.cmake)
add_imgui_and_sdl3(AudioPlay)
add_imgui_and_sdl3(AudioPlayTest)
target_link_libraries(AudioPlayTest PUBLIC SDL3_mixer::SDL3_mixer  PRIVATE SndFile::sndfile PRIVATE FFTW3)

target_link_libraries(AudioPlay PUBLIC SDL3_mixer::SDL3_mixer  PRIVATE SndFile::sndfile PRIVATE FFTW3 PRIVATE TagLib::tag PRIVATE TagLib::tag_c PRIVATE TagLib::TagLib)

configure_file(${CMAKE_CURRENT_LIST_DIR}/Assets/Icon.ico ${CMAKE_CURRENT_BINARY_DIR}/Assets/Icon.ico COPYONLY)