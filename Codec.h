
#ifndef AUDIOHIDER_CODEC_H
#define AUDIOHIDER_CODEC_H

#include <string>
#include <memory>
#include <functional>
#include "ffmpeg.h"

using std::string;

namespace FFmpeg{

    class frameDeletor{
    public:
        void operator()(AVFrame* f){
            av_frame_free(&f);
        }
    };
    class packetDeletor{
    public:
        void operator()(AVPacket *pkt){
            av_packet_free(&pkt);
        }

    };

    using UPacket = std::unique_ptr<AVPacket,packetDeletor>;
    using UFrame = std::unique_ptr<AVFrame,frameDeletor>;


    inline UPacket makeUPacket(){
        return UPacket(av_packet_alloc());
    }
    inline UFrame makeUFrame(){
        return UFrame{av_frame_alloc()};
    }
    inline UFrame makeUFrame(int format,int nb_samples,int channel_layout){
        auto f = av_frame_alloc();
        f->format = format;
        f->channel_layout =channel_layout;
        f->nb_samples= nb_samples;
        av_frame_get_buffer(f,0);
        return UFrame{f};
    }

    inline UFrame makeVideoUFrame(int format,int width,int height){
        auto f= av_frame_alloc();
        f->format = format;
        f->width= width;
        f->height= height;
        av_frame_get_buffer(f,0);
        return UFrame{f};
    }


    class Codec {
    public:
        enum class Type:char{Encoder,Decoder};

        AVCodec* codec;
    private:
        AVCodecContext* ctx{nullptr};

        Type type;

    public:
        explicit Codec(const string &name, Type t);
        explicit Codec(enum AVCodecID codecID, Type t);

        bool InitDecoder(const AVCodecParameters *par);
        bool InitVEncoder(int width,int height,int gop);
        bool InitAEncoder(int channels,int sampleRate,int bitRate);

        //for encoder
        void CopyToStream(AVCodecParameters *par) const;

        UPacket Encode(UFrame frame) noexcept ;
        UFrame Decode(UPacket packet) noexcept;
        ~Codec(){
            avcodec_free_context(&this->ctx);
        }
    };



    class AudioBuffer{
    private:
        AVAudioFifo *fifo{nullptr};
        SwrContext *converter{nullptr};
        int outChannel;
        AVSampleFormat  outSampleForamt;
        int outSampleRate;
        AVRational inTimeBase;
        int64_t firstStamp{0};
    public:
        AudioBuffer(int inChannels, AVSampleFormat inFormat, int inSampleRate, int outChannels,
                    AVSampleFormat outFormat, int outSampleRate, AVRational inTimeBase);
        AudioBuffer(const AudioBuffer &) = delete;
        AudioBuffer &operator=(AudioBuffer&) = delete;
        bool Write(UFrame frame);
        UFrame Read(int nb_samples);
        void Clean();
        ~AudioBuffer();
    };

    template<AVPixelFormat DST>
    class PixelConverter{
    private:
        SwsContext *ctx{nullptr};
        AVRational srcTimeBase{AV_TIME_BASE_Q};
    public:
        //PixelConverter(const PixelConverter &) = delete;
        //PixelConverter operator=(const PixelConverter &) = delete;
        void SetSrcTimeBase(AVRational src){ srcTimeBase = src;}

        UFrame operator()(UFrame && frame){
            if(!ctx){
                ctx = sws_getContext(frame->width, frame->height,(AVPixelFormat)frame->format, frame->width, frame->height, DST, SWS_BICUBIC, nullptr, nullptr, nullptr);
                if (!ctx) throw std::runtime_error("Can not create a SwsContext for video conversion");
            }
            auto f2 = makeVideoUFrame(DST,frame->width,frame->height);
            sws_scale(ctx, frame->data, frame->linesize, 0, frame->height,f2->data,f2->linesize);
            f2 ->pts = av_rescale_q(frame->pts,srcTimeBase,AV_TIME_BASE_Q);
            return std::move(f2);
        };
    };



}


#endif //AUDIOHIDER_CODEC_H
