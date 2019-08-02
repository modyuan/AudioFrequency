
#ifndef AUDIOHIDER_DEMUXER_H
#define AUDIOHIDER_DEMUXER_H

#include "Codec.h"

namespace FFmpeg{
    class Demuxer {
    public:
        virtual const AVCodecParameters *GetParams(int i) const = 0;
        virtual AVRational GetTimeBase(int i) const = 0;
        virtual int GetStreamCount() const = 0;
        virtual UPacket GetInput() = 0;
        virtual ~Demuxer() {};
    };



    class FileDemuxer:public Demuxer{
    private:
        AVFormatContext *inputFormatCtx {nullptr};
    public:
        explicit FileDemuxer(const string &filePath);
        FileDemuxer(const FileDemuxer&) = delete;
        const AVCodecParameters *GetParams(int i) const override {
            if(i>=0 && i<inputFormatCtx->nb_streams) return inputFormatCtx->streams[i]->codecpar;
            else return nullptr;
        }
        AVRational GetTimeBase(int i) const override{
            if(i>=0 && i<inputFormatCtx->nb_streams) return inputFormatCtx->streams[i]->time_base;
            else return {0,0};
        }
        int GetStreamCount() const override{return inputFormatCtx->nb_streams;}
        UPacket GetInput() override;
        ~FileDemuxer() override;
    };



    class InputDevice: public Demuxer{
    private:
        AVFormatContext *inputFormatCtx{nullptr};
    public:
        AVStream *stream{nullptr};

        explicit InputDevice(string name);
        UPacket GetInput() override ;
        AVRational GetTimeBase(int i) const override;
        const AVCodecParameters *GetParams(int i) const override;
        int GetStreamCount() const override {return 1;}
        ~InputDevice() override {
            avformat_close_input(&inputFormatCtx);
        }
    };
}


#endif //AUDIOHIDER_DEMUXER_H
