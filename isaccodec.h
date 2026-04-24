#ifndef ISAC_CODEC_H
#define ISAC_CODEC_H

#include "audiohandler.h"
#include "webrtc/modules/audio_coding/codecs/isac/main/interface/isac.h"

class ISACCodec {
private:
    ISACStruct* ISAC_main_inst;
    int16_t sampleRate;
    int32_t rateLimit;
    int32_t bottleneck = 50000;
    int16_t framesize = 30;
    int16_t payloadLimit = 300;

    int16_t speechType[1];

    int stream_len;

public:
    explicit ISACCodec(int sampleRate = 16000);  // ISAC supports 16kHz or 32kHz
    ~ISACCodec();

    bool encode(const std::vector<short>& input, std::vector<unsigned char>& encodedData);

    bool decode(const std::vector<unsigned char>& encodedData, std::vector<short>& decodedPcm);

    void setRateLimit(int bps);

    void setBottleneck(int bn);

    void setPayloadLimit(int pll);
};

#endif
