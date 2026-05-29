#include "audiohandler.h"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <algorithm>

// Более надежная структура для чтения WAV заголовков
struct WAVHeaderRaw {
    char riff[4];
    uint32_t fileSize;
    char wave[4];
    char fmt[4];
    uint32_t fmtSize;
};

// Функция для вывода отладочной информации
static void printDebug(const std::string& msg, bool verbose) {
    if (verbose) {
        std::cout << "[DEBUG] " << msg << std::endl;
    }
}

bool AudioHandler::readWAV(const std::string& filename,
                           std::vector<short>& samples,
                           int& sampleRate,
                           bool verbose) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        printDebug("Cannot open input file: " + filename, verbose);
        return false;
    }

    // Читаем базовый заголовок
    WAVHeaderRaw header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (!file || std::string(header.riff, 4) != "RIFF" ||
        std::string(header.wave, 4) != "WAVE" ||
        std::string(header.fmt, 4) != "fmt ") {
        printDebug("Invalid WAV file: incorrect RIFF/WAVE/fmt signature", verbose);
        return false;
    }

    printDebug("Found RIFF/WAVE/fmt chunks", verbose);
    printDebug("fmt chunk size: " + std::to_string(header.fmtSize), verbose);

    // Читаем fmt данные
    struct fmtData {
        uint16_t format;
        uint16_t channels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
    } fmt;

    file.read(reinterpret_cast<char*>(&fmt), sizeof(fmt));

    if (!file) {
        printDebug("Failed to read fmt chunk", verbose);
        return false;
    }

    printDebug("Format: " + std::to_string(fmt.format), verbose);
    printDebug("Channels: " + std::to_string(fmt.channels), verbose);
    printDebug("SampleRate: " + std::to_string(fmt.sampleRate), verbose);
    printDebug("BitsPerSample: " + std::to_string(fmt.bitsPerSample), verbose);

    if (fmt.format != 1) {
        printDebug("Only PCM WAV supported", verbose);
        return false;
    }

    if (fmt.channels != 1) {
        printDebug("Only mono WAV supported", verbose);
        return false;
    }

    if (fmt.bitsPerSample != 16) {
        printDebug("Only 16-bit WAV supported", verbose);
        return false;
    }

    sampleRate = fmt.sampleRate;

    // Пропускаем возможные дополнительные данные в fmt блоке
    if (header.fmtSize > sizeof(fmt)) {
        uint32_t extraSize = header.fmtSize - sizeof(fmt);
        printDebug("Skipping " + std::to_string(extraSize) + " bytes of extra fmt data", verbose);
        file.seekg(extraSize, std::ios::cur);
    }

    // Ищем data chunk
    char chunkName[4];
    uint32_t chunkSize;
    bool foundData = false;
    uint32_t dataSize = 0;

    while (file.read(chunkName, 4)) {
        file.read(reinterpret_cast<char*>(&chunkSize), 4);

        printDebug("Found chunk: " + std::string(chunkName, 4) +
                       " size: " + std::to_string(chunkSize), verbose);

        if (std::string(chunkName, 4) == "data") {
            dataSize = chunkSize;
            foundData = true;
            break;
        }

        // Пропускаем другие чанки
        file.seekg(chunkSize, std::ios::cur);
    }

    if (!foundData) {
        printDebug("Data chunk not found", verbose);
        return false;
    }

    // Проверяем, что размер данных кратен размеру выборки
    size_t expectedSamples = dataSize / fmt.blockAlign;
    samples.resize(expectedSamples);

    printDebug("Reading " + std::to_string(dataSize) + " bytes of audio data", verbose);
    printDebug("Expected samples: " + std::to_string(expectedSamples), verbose);

    file.read(reinterpret_cast<char*>(samples.data()), dataSize);

    size_t bytesRead = file.gcount();
    printDebug("Actually read: " + std::to_string(bytesRead) + " bytes", verbose);

    if (bytesRead < dataSize) {
        printDebug("Warning: read fewer bytes than expected", verbose);
        samples.resize(bytesRead / fmt.blockAlign);
        return false;
    }

    printDebug("Successfully loaded " + std::to_string(samples.size()) + " samples", verbose);
    return true;
}

bool AudioHandler::writeWAV(const std::string& filename,
                            const std::vector<short>& samples,
                            int sampleRate,
                            bool verbose) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        printDebug("Cannot create output file: " + filename, verbose);
        return false;
    }

    printDebug("Writing WAV file: " + filename, verbose);
    printDebug("Samples: " + std::to_string(samples.size()), verbose);
    printDebug("SampleRate: " + std::to_string(sampleRate), verbose);

    uint32_t dataSize = static_cast<uint32_t>(samples.size() * sizeof(short));
    uint32_t fileSize = 36 + dataSize; // 44 bytes header + data

    // Записываем заголовок
    file.write("RIFF", 4);
    file.write(reinterpret_cast<const char*>(&fileSize), 4);
    file.write("WAVE", 4);
    file.write("fmt ", 4);

    uint32_t fmtSize = 16;
    uint16_t format = 1; // PCM
    uint16_t channels = 1; // mono
    uint16_t bitsPerSample = 16;
    uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
    uint16_t blockAlign = channels * (bitsPerSample / 8);

    file.write(reinterpret_cast<const char*>(&fmtSize), 4);
    file.write(reinterpret_cast<const char*>(&format), 2);
    file.write(reinterpret_cast<const char*>(&channels), 2);
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    file.write("data", 4);
    file.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Записываем аудио данные
    file.write(reinterpret_cast<const char*>(samples.data()), dataSize);

    printDebug("Wrote " + std::to_string(dataSize) + " bytes of audio data", verbose);
    printDebug("WAV file written successfully", verbose);

    return file.good();
}

WAVInfo AudioHandler::validateWAV(const std::string& filename, bool verbose) {
    WAVInfo info = {0, 0, 0, 0, ""};

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        info.errorMsg = "Cannot open file: " + filename;
        printDebug(info.errorMsg, verbose);
        return info;
    }

    WAVHeaderRaw header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (std::string(header.riff, 4) != "RIFF") {
        info.errorMsg = "Invalid RIFF header";
        printDebug(info.errorMsg, verbose);
        return info;
    }

    if (std::string(header.wave, 4) != "WAVE") {
        info.errorMsg = "Invalid WAVE header";
        printDebug(info.errorMsg, verbose);
        return info;
    }

    // Читаем fmt
    struct fmtData {
        uint16_t format;
        uint16_t channels;
        uint32_t sampleRate;
        uint32_t byteRate;
        uint16_t blockAlign;
        uint16_t bitsPerSample;
    } fmt;

    file.read(reinterpret_cast<char*>(&fmt), sizeof(fmt));

    info.channels = fmt.channels;
    info.sampleRate = fmt.sampleRate;
    info.bitsPerSample = fmt.bitsPerSample;

    printDebug("Validation results for " + filename, verbose);
    printDebug("  Sample Rate: " + std::to_string(info.sampleRate) + " Hz", verbose);
    printDebug("  Channels: " + std::to_string(info.channels), verbose);
    printDebug("  Bits per sample: " + std::to_string(info.bitsPerSample), verbose);

    return info;
}

bool AudioHandler::compareWAVs(const std::string& filename1,
                               const std::string& filename2,
                               double tolerancePercent) {
    std::vector<short> samples1, samples2;
    int sampleRate1, sampleRate2;

    if (!readWAV(filename1, samples1, sampleRate1, false)) {
        std::cerr << "Failed to read first WAV file" << std::endl;
        return false;
    }

    if (!readWAV(filename2, samples2, sampleRate2, false)) {
        std::cerr << "Failed to read second WAV file" << std::endl;
        return false;
    }

    if (sampleRate1 != sampleRate2) {
        std::cerr << "Sample rates differ: " << sampleRate1 << " vs " << sampleRate2 << std::endl;
        return false;
    }

    if (samples1.size() != samples2.size()) {
        std::cerr << "Sample counts differ: " << samples1.size() << " vs " << samples2.size() << std::endl;
        return false;
    }

    double maxDiff = 0;
    long long totalDiff = 0;

    for (size_t i = 0; i < samples1.size(); ++i) {
        double diff = std::abs(samples1[i] - samples2[i]);
        maxDiff = std::max(maxDiff, diff);
        totalDiff += diff;
    }

    double avgDiff = static_cast<double>(totalDiff) / samples1.size();
    double maxPercent = (maxDiff / 32768.0) * 100.0;

    std::cout << "Comparison results:" << std::endl;
    std::cout << "  Max difference: " << maxDiff << " (" << std::fixed << std::setprecision(2)
              << maxPercent << "%)" << std::endl;
    std::cout << "  Average difference: " << avgDiff << std::endl;

    if (tolerancePercent > 0 && maxPercent > tolerancePercent) {
        std::cout << "  FAIL: Differences exceed tolerance of " << tolerancePercent << "%" << std::endl;
        return false;
    }

    if (maxDiff == 0) {
        std::cout << "  SUCCESS: Files are identical" << std::endl;
    } else {
        std::cout << "  SUCCESS: Files are within tolerance" << std::endl;
    }

    return true;
}
