cmake_minimum_required(VERSION 3.10)
project(AudioFrequency)

set(CMAKE_CXX_STANDARD 14)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-D_DEBUG)
endif()


if(WIN32)
    link_directories("C:\\LIB\\ffmpeg-4.1.4-win64-shared\\lib")
    include_directories("C:\\LIB\\ffmpeg-4.1.4-win64-shared\\include")
elseif(APPLE)
    link_directories(/usr/local/lib)
    include_directories(/usr/local/include)
endif()

set(lib avcodec avdevice avfilter avformat avutil postproc swresample swscale)

set(src main.cpp AudioFFT.cpp FrequencySpectrum.cpp EnumDevice.cpp Demuxer.cpp Codec.cpp utils.cpp )

if(WIN32)
    set(src ${src} EnumDevice.cpp)
    set(lib ${lib} Strmiids)
elseif(APPLE)
    set(src ${src} EnumDevice.mm)
    set(lib ${lib} "-framework AVFoundation" "-framework corevideo" "-framework applicationservices")
endif()

add_executable(AudioFrequency ${src})
target_link_libraries(AudioFrequency ${lib})

