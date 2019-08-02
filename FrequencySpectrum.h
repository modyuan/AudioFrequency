#ifndef MEDIAPLAYER_FREQUENCYSPECTRUM_H
#define MEDIAPLAYER_FREQUENCYSPECTRUM_H

#include <vector>
#include <cstdint>
#include <math.h>
#include "AudioFFT.h"

template<typename T>
class FrequencySpectrum {
private:
    int sampleRate;
    int bufferSize;
    std::vector<float> buffer;
    audiofft::AudioFFT fft;
    static std::vector<float> GetLength(std::vector<float> &v1,std::vector<float> &v2){
        std::vector<float> r;
        for(int i=0;i<(int)v1.size();i++){
            float t = sqrtf(v1[i]*v1[i] + v2[i]*v2[i]);
            r.push_back(t);
        }
        return r;
    }
public:

    explicit FrequencySpectrum(int sampleRate):sampleRate(sampleRate) {
        bufferSize = 2;
        while(bufferSize*2 < sampleRate) bufferSize *=2;
        buffer = std::vector<float>(bufferSize,0);
        fft.init(bufferSize);
    }
    float GetMaxF(){
        int size = bufferSize;
        std::vector<float> re(audiofft::AudioFFT::ComplexSize(size));
        std::vector<float> vi(audiofft::AudioFFT::ComplexSize(size));
        fft.fft(buffer.data(),re.data(),vi.data());
        auto result = GetLength(re,vi);
        float t = 0;
        int fr = 0;
        for(int i=0;i<result.size();i++){
            if(result[i]>t){
                t = result[i];
                fr = i;
            }
        }
        return (float)fr/bufferSize*sampleRate;
    }
    std::vector<float> Get(){
        int size = sampleRate;
        std::vector<float> re(audiofft::AudioFFT::ComplexSize(size));
        std::vector<float> vi(audiofft::AudioFFT::ComplexSize(size));
        fft.fft(buffer.data(),re.data(),vi.data());
        return GetLength(re,vi);
    }
    void Push(T* sample,int count,int step){
        std::vector<float> b1;
        if(count > bufferSize) printf("too much!!\n");
        for(int i = count;i<buffer.size();i++){
            b1.push_back(buffer[i]);
        }
        for(int i=0;i<count;i++){
            b1.push_back((float)(*sample));
            sample+=step;
        }
        //printf("count: %d\n",count);
        buffer = b1;
    }

};



#endif //MEDIAPLAYER_FREQUENCYSPECTRUM_H
