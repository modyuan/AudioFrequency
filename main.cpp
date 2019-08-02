#include <vector>
#include <string>

#include "Demuxer.h"
#include "Codec.h"
#include "EnumDevice.h"
#include "FrequencySpectrum.h"
#include "utils.h"

using namespace std;
using namespace FFmpeg;


int main() {
    auto devices = GetDevices('a');
    if(devices.empty()){
        fprintf(stderr,"No audio input device!\n");
        return -1;
    }
    InputDevice input{devices[0].inputName};
    auto par = input.GetParams(0);
    Codec decoder{par->codec_id,Codec::Type::Decoder};
    decoder.InitDecoder(par);

    AudioBuffer buffer(par->channels,(AVSampleFormat)par->format,par->sample_rate,
            1,AV_SAMPLE_FMT_FLT,par->sample_rate,input.GetTimeBase(0));

    FrequencySpectrum<float> fs{par->sample_rate};


    while(true){
        auto pkt = input.GetInput();
        if(!pkt) {
            puts("EOF");
            break;
        }
        auto frame = decoder.Decode(std::move(pkt));
        if(!frame) continue;

        int s = frame->nb_samples;
        buffer.Write(std::move(frame));
        frame = buffer.Read(s);

        fs.Push((float*)frame->data[0],frame->nb_samples,1);
        int f = (int)fs.GetMaxF();
        printf("fr:%7d Hz\n",f);

    }

    return 0;

}



