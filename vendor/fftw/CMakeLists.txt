# Задаем версию FFTW
set(FFTW_VERSION 3.3.10)
set(FFTW_URL https://fftw.org/fftw-${FFTW_VERSION}.tar.gz)

# Директория для скачивания и распаковки FFTW
set(FFTW_SOURCE_DIR ${CMAKE_BINARY_DIR}/fftw-${FFTW_VERSION})

# Функция для скачивания и распаковки архива
include(FetchContent)
FetchContent_Declare(
    fftw
    URL ${FFTW_URL}
)

# Распаковываем архив и подключаем его как субдиректорию
FetchContent_GetProperties(fftw)
if(NOT fftw_POPULATED)
    FetchContent_Populate(fftw)
endif()
add_subdirectory(${fftw_SOURCE_DIR} ${CMAKE_BINARY_DIR}/fftw-build)
add_library(FFTW3 INTERFACE)
target_link_libraries(FFTW3 INTERFACE fftw3)
target_include_directories(FFTW3 INTERFACE ${fftw_SOURCE_DIR}/api)