#ifdef __APPLE__

#include "EnumDevice.h"
#import <AVFoundation/AVFoundation.h>

using std::string;
using std::vector;

// flags: -framework AVFoundation -framework corevideo -framework applicationservices

// copy from FFmpeg and modified
vector<DeviceName> GetDevices(char type){

    vector<DeviceName> result;

    @autoreleasepool {

        if(type == 'v' || type == 'V'){

            NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
            uint32_t num_screens = 0;
#if !TARGET_OS_IPHONE && __MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
            CGGetActiveDisplayList(0, NULL, &num_screens);
#endif
            int index = 0;
            for (AVCaptureDevice *device in devices) {
                const char *name = [[device localizedName] UTF8String];
                index            = (int)[devices indexOfObject:device];
                char index_str[10]={0};
                snprintf(index_str,10,"%d",index);
                result.push_back({string(name),string(index_str)});
                index++;
            }
#if !TARGET_OS_IPHONE && __MAC_OS_X_VERSION_MIN_REQUIRED >= 1070
            if (num_screens > 0) {
                CGDirectDisplayID screens[num_screens];
                CGGetActiveDisplayList(num_screens, screens, &num_screens);
                for (int i = 0; i < num_screens; i++) {
                    char name_str[30] = {0};
                    char index_str[10] = {0};
                    snprintf(name_str,30,"Capture screen %d",i);
                    snprintf(index_str,10,"%d",index + i);
                    result.push_back({string(name_str),string(index_str)});
                }
            }
#endif
        }else if(type=='a' || type == 'A'){
            NSArray *devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeAudio];
            for (AVCaptureDevice *device in devices) {
                const char *name = [[device localizedName] UTF8String];
                int index  = (int)[devices indexOfObject:device];
                char index_str[10] = {0};
                snprintf(index_str,10,":%d",index);
                result.push_back({string(name),string(index_str)});
            }
        }
    }
    return result;
}


#endif // __APPLE__