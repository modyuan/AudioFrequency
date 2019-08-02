
#include "Codec.h"
#include "utils.h"
#include <string>
#include <stdexcept>

namespace FFmpeg {
    Codec::Codec(const string &name, Type t) : type(t) {
        if (t == Type::Encoder) {
            codec = avcodec_find_encoder_by_name(name.c_str());
        } else {
            codec = avcodec_find_decoder_by_name(name.c_str());
        }
        if (!codec) {
            throw std::runtime_error("Can not find codec");
        }
    }

    Codec::Codec(enum AVCodecID codecID, Type t) : type(t) {
        if (t == Type::Encoder) {
            codec = avcodec_find_encoder(codecID);
        } else {
            codec = avcodec_find_decoder(codecID);
        }
        if (!codec) {
            throw std::runtime_error("Can not find codec");
        }
    }

    bool Codec::InitDecoder(const AVCodecParameters *par) {
        ctx = avcodec_alloc_context3(codec);
        if (!ctx) {
            throw std::runtime_error("Fail to alloc CodecContext");
        }
        if (par) {
            avcodec_parameters_to_context(ctx, par);
        }
        int ret = avcodec_open2(ctx, codec, nullptr);
        if (ret < 0) {
            avcodec_free_context(&ctx);
            throw std::runtime_error("Could not open video codec.");
        }
        return true;

    }

    UPacket Codec::Encode(UFrame frame) noexcept {
        int ret = avcodec_send_frame(ctx, frame.get());
        if (ret < 0) {
            CAVERR("Codec", "Fail to Encode Frame(1)");
            return {};
        }
        auto pkt = makeUPacket();
        ret = avcodec_receive_packet(ctx, pkt.get());
        if (ret < 0) {
            if (ret != AVERROR(EAGAIN)) {
                CAVERR("Codec", "Fail to Encode Frame(2)");
            }
            return {};
        }
        return pkt;
    }

    UFrame Codec::Decode(UPacket packet) noexcept {
        int ret = avcodec_send_packet(ctx, packet.get());
        if (ret < 0) {
            CAVERR("Codec", "Fail to Decode packet(1)");
            return {};
        }
        auto frame = makeUFrame();
        ret = avcodec_receive_frame(ctx, frame.get());
        if (ret < 0) {
            if (ret != AVERROR(EAGAIN)) {
                CAVERR("Codec", "Fail to Decode packet(2)");
            }
            return {};
        }
        return frame;
    }

    bool Codec::InitAEncoder(int channels, int sampleRate, int bitRate) {
        ctx = avcodec_alloc_context3(codec);
        if (!ctx) {
            throw std::runtime_error("Fail to alloc CodecContext");
        }
        ctx->channels = channels;
        ctx->channel_layout = av_get_default_channel_layout(channels);
        ctx->sample_rate = sampleRate;
        ctx->bit_rate = bitRate;
        ctx->sample_fmt = AV_SAMPLE_FMT_S16;
        int ret = avcodec_open2(ctx, codec, nullptr);
        if (ret < 0) {
            avcodec_free_context(&ctx);
            throw std::runtime_error("Could not open codec.");
        }
        return true;
    }

    bool Codec::InitVEncoder(int width, int height, int gop) {
        return false;
    }

    void Codec::CopyToStream(AVCodecParameters *par) const {
        avcodec_parameters_from_context(par, ctx);
    }


    AudioBuffer::AudioBuffer(int inChannels, AVSampleFormat inFormat, int inSampleRate, int outChannel,
                             AVSampleFormat outFormat, int outSampleRate, AVRational timeBase)
            : outChannel(outChannel), outSampleForamt(outFormat), outSampleRate(outSampleRate), inTimeBase(timeBase) {
        converter = swr_alloc_set_opts(nullptr, av_get_default_channel_layout(outChannel), (AVSampleFormat) outFormat,
                                       outSampleRate,
                                       av_get_default_channel_layout(inChannels), (AVSampleFormat) inFormat,
                                       inSampleRate,
                                       0, nullptr);

        int ret = swr_init(converter);
        if (ret < 0) {
            swr_free(&converter);
            throw std::runtime_error("Fail to init swr");
        }
        fifo = av_audio_fifo_alloc(outFormat, outChannel, outSampleRate * 2);
        if (!fifo) {
            swr_free(&converter);
            throw std::runtime_error("Fail to alloc audio fifo");
        }

    }

    bool AudioBuffer::Write(UFrame frame) {
        if (!frame) {
            CAVERR("AudioBuffer", "input frame is null");
            return false;
        }

        int upperBoundNum = swr_get_out_samples(converter,
                                                frame->nb_samples); // upperBoundNum is not a accurate number, it's a upper bound
        if (av_audio_fifo_space(fifo) < upperBoundNum) {
            return false;
        }

        uint8_t **convertBuffer = nullptr;
        av_samples_alloc_array_and_samples(&convertBuffer, nullptr, outChannel, upperBoundNum, outSampleForamt, 0);

        int num = swr_convert(converter, convertBuffer, upperBoundNum, (const uint8_t **) frame->extended_data,
                              frame->nb_samples);

        if (num < 0) {
            CAVERR("AudioBuffer", "Fail to convert samples");
            av_freep(&convertBuffer[0]);
            return false;
        }
        int ret = av_audio_fifo_write(fifo, (void **) convertBuffer, num);
        if (ret < 0) {
            CAVERR("AudioBuffer", "Fail to write samples to fifo");
            av_freep(&convertBuffer[0]);
            return false;
        }
        av_freep(&convertBuffer[0]);

        int ss = av_audio_fifo_size(fifo);
        firstStamp = av_rescale_q(frame->pts, inTimeBase, AV_TIME_BASE_Q);
        firstStamp -= av_audio_fifo_size(fifo) * AV_TIME_BASE / outSampleRate;
        return true;
    }

    UFrame AudioBuffer::Read(int nb_samples) {
        int size = av_audio_fifo_size(fifo);
        if (size >= nb_samples) {
            auto frame = makeUFrame(outSampleForamt, nb_samples, av_get_default_channel_layout(outChannel));
            av_audio_fifo_read(fifo, (void **) frame->data, nb_samples);
            frame->pts = firstStamp;
            firstStamp += nb_samples * AV_TIME_BASE / outSampleRate;
            return frame;
        } else {
            return {};
        }
    }

    AudioBuffer::~AudioBuffer() {
        swr_free(&converter);
        av_audio_fifo_free(fifo);
    }

    void AudioBuffer::Clean() {
        av_audio_fifo_reset(fifo);
    }




}
