
#include "Demuxer.h"
#include "EnumDevice.h"
#include <stdio.h>

namespace FFmpeg{
    // -------------------------------------------------
    // Demuxer

    FileDemuxer::FileDemuxer(const string &filePath) {
        int ret = avformat_open_input(&inputFormatCtx,filePath.c_str(),nullptr,nullptr);
        if(ret <0){
            throw std::runtime_error("Fail to open file");
        }

        ret = avformat_find_stream_info(inputFormatCtx,nullptr);
        if(ret<0){
            avformat_close_input(&inputFormatCtx);
            throw std::runtime_error("Fail to find stream info");
        }

    }

    FileDemuxer::~FileDemuxer() {
        avformat_close_input(&inputFormatCtx);
    }

    UPacket FileDemuxer::GetInput() {
        auto pkt = makeUPacket();
        int ret = av_read_frame(inputFormatCtx, pkt.get());
        if (ret < 0) {
            return {};
        } else {
            return pkt;
        }
    }




    InputDevice::InputDevice(string name) {

        avdevice_register_all();
        //for Windows
        AVDictionary *dict = nullptr;
        AVInputFormat *inputFmt = av_find_input_format( INPUT_FORMAT_NAME );

        if(name == "a" || name == "v"){
            auto t = GetDevices(name[0]);
            if(t.empty()) throw::std::runtime_error("can not detect input device");
            name = t[0].inputName;
        }

        int ret = avformat_open_input(&inputFormatCtx, name.c_str(), inputFmt, nullptr);
        if (ret < 0) {
            throw std::runtime_error("Fail to open input device");
        }

        ret = avformat_find_stream_info(inputFormatCtx, nullptr);
        if (ret < 0) {
            avformat_close_input(&inputFormatCtx);
            throw std::runtime_error("Can not find stream info");
        }

        for (auto i = 0; i < inputFormatCtx->nb_streams; ++i) {
            AVMediaType t = inputFormatCtx->streams[i]->codecpar->codec_type;
            if (t == AVMEDIA_TYPE_AUDIO || t == AVMEDIA_TYPE_VIDEO) {
                stream = inputFormatCtx->streams[i];
                break;
            }
        }
        if (!stream) {
            avformat_close_input(&inputFormatCtx);
            throw std::runtime_error("Can not find stream");
        }

    }

    UPacket InputDevice::GetInput() {
        auto pkt = makeUPacket();
        int ret = av_read_frame(inputFormatCtx, pkt.get());
        if (ret < 0) {
            return {};
        } else {
            return pkt;
        }
    }

    const AVCodecParameters *InputDevice::GetParams(int i) const {
        return stream->codecpar;
    }

    AVRational InputDevice::GetTimeBase(int i) const {
        return stream->time_base;
    }


}