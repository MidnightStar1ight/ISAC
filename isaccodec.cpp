#include "isaccodec.h"
#include <cstring>
#include <iostream>

ISACCodec::ISACCodec(int sampleRate):
    sampleRate(sampleRate),
    samples10ms(sampleRate / 100) {

    this->sampleRate = sampleRate;
    this->rateLimit = (sampleRate == 16000) ? 53400 : 106800;

    if (sampleRate == 16000 or sampleRate == 32000) {
        WebRtcIsac_Create(&ISAC_main_inst);
        WebRtcIsac_SetEncSampRate(ISAC_main_inst, sampleRate);
        WebRtcIsac_SetDecSampRate(ISAC_main_inst, sampleRate);
    }
    else {
        throw std::invalid_argument("Unsupported sample rate (should be 16kHz or 32kHz");
    }

    if (WebRtcIsac_EncoderInit(ISAC_main_inst, 1) < 0) {
        throw std::runtime_error("Failed to initialise encoder");
    }
    if (WebRtcIsac_DecoderInit(ISAC_main_inst) < 0)
        throw std::runtime_error("Failed to initialise decoder");

    if (WebRtcIsac_Control(ISAC_main_inst, bottleneck, framesize) < 0) {
        throw std::runtime_error("Failed to set bottleneck");
    }

    if (WebRtcIsac_SetMaxPayloadSize(ISAC_main_inst, payloadLimit) < 0) {
        throw std::runtime_error("Failed to set max payload size");
    }

    if (WebRtcIsac_SetMaxRate(ISAC_main_inst, rateLimit) < 0) {
        throw std::runtime_error("Failed to set max rate");
    }

    encoderBuffer.reserve(samples10ms * 3);  // Для 30ms фрейма

}

ISACCodec::~ISACCodec() {
    if (ISAC_main_inst) {
        WebRtcIsac_Free(ISAC_main_inst);
    }
}

void ISACCodec::setRateLimit(int bps) {
    if (ISAC_main_inst) {
        this->rateLimit = bps;
    }
}

void ISACCodec::setBottleneck(int bn)
{
    if (ISAC_main_inst) {
        this->bottleneck = bn;
    }
}

void ISACCodec::setPayloadLimit(int pll)
{
    if (ISAC_main_inst) {
        this->payloadLimit = pll;
    }
}

// bool ISACCodec::encode(const std::vector<short>& input, std::vector<unsigned char>& encodedData) {
//     // 30ms frames: 16kHz = 480 samples, 32kHz = 960 samples
//     const int frameLen = (sampleRate == 16000) ? 480 : 960;
//     if (input.size() != frameLen) {
//         std::cout << frameLen << std::endl;
//         std::cerr << "Input must be exactly " << frameLen << " samples for ISAC encoding\n";
//         return false;
//     }

//     this->stream_len = WebRtcIsac_Encode(ISAC_main_inst, input.data(), encodedData.data());
//     if (stream_len <= 0) {
//         std::cerr << "ISAC encoding failed (stream_len <= 0)\n";
//         return false;
//     }

//     encodedData.resize(stream_len);
//     return true;
// }

bool ISACCodec::encode(const std::vector<short>& input,
                       std::vector<unsigned char>& encodedData) {
    const int samples10ms = sampleRate / 100;

    if (input.size() != samples10ms) {
        std::cerr << "Input must be 10ms frame (" << samples10ms
                  << " samples), got " << input.size() << "\n";
        return false;
    }

    // Передаем 10ms фрейм напрямую в ISACw
    encodedData.resize(2000);
    int stream_len = WebRtcIsac_Encode(ISAC_main_inst,
                                       input.data(),
                                       encodedData.data());

    if (stream_len > 0) {
        // ISAC накопил достаточно данных и закодировал 30ms фрейм
        encodedData.resize(stream_len);
        // std::cout << "Encoded to " << stream_len << " bytes\n";
        return true;
    } else if (stream_len == 0) {
        // Нужно больше данных
        encodedData.clear();
        return false;  // Ждем следующие 10ms
    } else {
        std::cerr << "ISAC encoding error: " << stream_len << "\n";
        return false;
    }
}

// bool ISACCodec::decode(const std::vector<unsigned char>& encodedData, std::vector<short>& decodedPcm) {
//     const int frameLen = (sampleRate == 16000) ? 480 : 960;
//     decodedPcm.resize(frameLen);

//     int samplesOut = WebRtcIsac_Decode(ISAC_main_inst, encodedData.data(), stream_len, decodedPcm.data(), speechType);
//     if (samplesOut != frameLen) {
//         std::cerr << "ISAC decoding failed or incomplete (samplesOut != frameLen)\n";
//         return false;
//     }

//     return true;
// }

bool ISACCodec::decode(const std::vector<unsigned char>& encodedData,
                       std::vector<short>& decodedPcm) {

    // Временный буфер максимального размера (60ms = 960 семплов при 16kHz)
    decodedPcm.resize(960);  // Максимальный возможный размер

    int samplesOut = WebRtcIsac_Decode(ISAC_main_inst,
                                       encodedData.data(),
                                       static_cast<int16_t>(encodedData.size()),
                                       decodedPcm.data(),
                                       speechType);

    if (samplesOut <= 0) {
        std::cerr << "ISAC decoding failed, samplesOut=" << samplesOut << "\n";
        return false;
    }

    // Обрезаем до реального количества декодированных семплов
    decodedPcm.resize(samplesOut);

    // std::cout << "Decoded " << samplesOut << " samples ("
    //           << (samplesOut * 1000.0 / sampleRate) << " ms)\n";

    return true;
}
