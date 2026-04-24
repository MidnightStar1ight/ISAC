#include "audiohandler.h"
#include <fstream>
#include <iostream>

struct WAVHeader {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;       // 16 for PCM
    uint16_t format;        // 1 = PCM
    uint16_t channels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char data[4];
    uint32_t dataSize;
};

bool AudioHandler::readWAV(const std::string& filename, std::vector<short>& samples, int& sampleRate) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot open input file: " << filename << "\n";
        return false;
    }

    WAVHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (!file || std::string(header.riff, 4) != "RIFF" || std::string(header.wave, 4) != "WAVE") {
        std::cerr << "Invalid WAV file\n";
        return false;
    }

    if (header.format != 1) {
        std::cerr << "Only PCM WAV supported\n";
        return false;
    }

    if (header.channels != 1) {
        std::cerr << "Only mono WAV supported\n";
        return false;
    }

    sampleRate = header.sampleRate;

    samples.resize(header.dataSize / sizeof(short));
    file.read(reinterpret_cast<char*>(samples.data()), header.dataSize);

    return true;
}

bool AudioHandler::writeWAV(const std::string& filename, const std::vector<short>& samples, int sampleRate) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Cannot create output file: " << filename << "\n";
        return false;
    }

    WAVHeader header{};
    memcpy(header.riff, "RIFF", 4);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    memcpy(header.data, "data", 4);

    header.fmtSize = 16;
    header.format = 1;
    header.channels = 1;
    header.sampleRate = sampleRate;
    header.bitsPerSample = 16;
    header.blockAlign = 2;
    header.byteRate = sampleRate * 2;
    header.dataSize = static_cast<uint32_t>(samples.size() * sizeof(short));
    header.fileSize = 36 + header.dataSize;

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(samples.data()), samples.size() * sizeof(short));

    return true;
}
