#ifdef _WIN32 // in windows

#include "EnumDevice.h"
#include <string>
#include <stdexcept>

#define UNICODE
extern "C"{
#include <windows.h>
#include <dshow.h>
};

#ifndef MACRO_GROUP_DEVICENAME
#define MACRO_GROUP_DEVICENAME
#define MAX_FRIENDLY_NAME_LENGTH	128
#define MAX_MONIKER_NAME_LENGTH		256
#endif

using namespace std;

typedef struct _TDeviceName
{
    WCHAR FriendlyName[MAX_FRIENDLY_NAME_LENGTH];	// 设备友好名
    WCHAR MonikerName[MAX_MONIKER_NAME_LENGTH];		// 设备Moniker名
} TDeviceName;

std::string unicode2utf8(const WCHAR* uni) {
    static char temp[500];// max length of friendly name by UTF-16 is 128, so 500 in enough by utf-8
    memset(temp, 0, 500);
    WideCharToMultiByte(CP_UTF8, 0, uni, -1, temp, 500, NULL, NULL);
    return std::string(temp);
}

vector<DeviceName> GetDevices(const char deviceType)
{
    GUID guidValue;
    const char * prefix;
    if (deviceType == 'v' || deviceType == 'V') {
        prefix = "video=";
        guidValue = CLSID_VideoInputDeviceCategory;
    }
    else if (deviceType == 'a' || deviceType == 'A') {
        prefix = "audio=";
        guidValue = CLSID_AudioInputDeviceCategory;
    }
    else {
        throw std::invalid_argument("param deviceType must be 'a' or 'v'.");
    }


    WCHAR FriendlyName[MAX_FRIENDLY_NAME_LENGTH];
    HRESULT hr;

    // 初始化
    vector<DeviceName> vectorDevices;

    // 初始化COM
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        return {};
    }

    // 创建系统设备枚举器实例
    ICreateDevEnum *pSysDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
    if (FAILED(hr))
    {
        CoUninitialize();
        return {};
    }

    // 获取设备类枚举器
    IEnumMoniker *pEnumCat = NULL;
    hr = pSysDevEnum->CreateClassEnumerator(guidValue, &pEnumCat, 0);
    if (hr == S_OK)
    {
        // 枚举设备名称
        IMoniker *pMoniker = NULL;
        ULONG cFetched;
        while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
        {
            IPropertyBag *pPropBag;
            hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void **)&pPropBag);
            if (SUCCEEDED(hr))
            {
                // 获取设备友好名
                VARIANT varName;
                VariantInit(&varName);

                hr = pPropBag->Read(L"FriendlyName", &varName, NULL);
                if (SUCCEEDED(hr))
                {
                    StringCchCopyW(FriendlyName, MAX_FRIENDLY_NAME_LENGTH, varName.bstrVal);
                    string displayName = unicode2utf8(FriendlyName);
                    vectorDevices.push_back({displayName, prefix + displayName});
                }

                VariantClear(&varName);
                pPropBag->Release();
            }

            pMoniker->Release();
        } // End for While

        pEnumCat->Release();
    }

    pSysDevEnum->Release();
    CoUninitialize();

    return vectorDevices;
}
#endif // _WIN32
