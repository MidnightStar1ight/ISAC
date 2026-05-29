#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include <string>
#include <vector>

struct WAVInfo {
    int sampleRate;
    int channels;
    int bitsPerSample;
    int samplesCount;
    std::string errorMsg;
};

class AudioHandler {
public:
    static bool readWAV(const std::string& filename,
                        std::vector<short>& samples,
                        int& sampleRate,
                        bool verbose = false); // verbose для вывода отладки

    static bool writeWAV(const std::string& filename,
                         const std::vector<short>& samples,
                         int sampleRate,
                         bool verbose = false);

    // Дополнительная функция для проверки WAV файла без загрузки данных
    static WAVInfo validateWAV(const std::string& filename, bool verbose = false);

    // Функция для сравнения двух WAV файлов
    static bool compareWAVs(const std::string& filename1,
                            const std::string& filename2,
                            double tolerancePercent = 0.0);
};

#endif
