#ifndef LIBCAV_ENUMDEVICE_H
#define LIBCAV_ENUMDEVICE_H

// this function can only be used in windows

#include <vector>
#include <string>

struct DeviceName{
    std::string displayName; // use for show in display, in utf-8
    std::string inputName; // use in ffmpeg, in utf-8
};


std::vector<DeviceName> GetDevices(char type);

#ifdef _WIN32
#define INPUT_FORMAT_NAME "dshow"
#elif __APPLE__
#define INPUT_FORMAT_NAME "avfoundation"
#endif


#endif //LIBCAV_ENUMDEVICE_H
