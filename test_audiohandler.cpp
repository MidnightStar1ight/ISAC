// #include "audiohandler.h"
// #include <iostream>
// #include <cmath>
// #include <chrono>

// // Функция для создания тестового сигнала (синусоида)
// std::vector<short> generateSineWave(int sampleRate, int durationSec, double frequencyHz) {
//     std::vector<short> samples;
//     int totalSamples = sampleRate * durationSec;
//     samples.reserve(totalSamples);

//     for (int i = 0; i < totalSamples; ++i) {
//         double time = static_cast<double>(i) / sampleRate;
//         double value = sin(2.0 * M_PI * frequencyHz * time);
//         short sample = static_cast<short>(value * 32767.0);
//         samples.push_back(sample);
//     }

//     return samples;
// }

// // Функция для вывода статистики аудио
// void printAudioStats(const std::vector<short>& samples, const std::string& name) {
//     if (samples.empty()) {
//         std::cout << name << ": No samples" << std::endl;
//         return;
//     }

//     long long sum = 0;
//     short minSample = samples[0];
//     short maxSample = samples[0];

//     for (short sample : samples) {
//         sum += std::abs(sample);
//         minSample = std::min(minSample, sample);
//         maxSample = std::max(maxSample, sample);
//     }

//     double avgAbs = static_cast<double>(sum) / samples.size();

//     std::cout << "\n=== Audio Statistics: " << name << " ===" << std::endl;
//     std::cout << "Sample count: " << samples.size() << std::endl;
//     std::cout << "Min sample: " << minSample << std::endl;
//     std::cout << "Max sample: " << maxSample << std::endl;
//     std::cout << "Average absolute value: " << avgAbs << std::endl;
//     std::cout << "Peak-to-peak: " << (maxSample - minSample) << std::endl;
//     std::cout << "First 10 samples: ";
//     for (int i = 0; i < std::min(10, (int)samples.size()); ++i) {
//         std::cout << samples[i] << " ";
//     }
//     std::cout << std::endl;
// }

// // Основная тестовая функция
// void testWAVReadWrite() {
//     std::cout << "=== Starting WAV Read/Write Test ===" << std::endl;

//     const int sampleRate = 44100;
//     const int durationSec = 2;
//     const double frequency = 440.0; // нота A

//     // 1. Генерируем тестовый сигнал
//     std::cout << "\n1. Generating test sine wave (440Hz, 2 seconds, 44.1kHz)..." << std::endl;
//     std::vector<short> originalSamples = generateSineWave(sampleRate, durationSec, frequency);
//     printAudioStats(originalSamples, "Generated Signal");

//     // 2. Сохраняем в WAV с отладкой
//     std::cout << "\n2. Saving to test.wav with debug output..." << std::endl;
//     if (AudioHandler::writeWAV("test.wav", originalSamples, sampleRate, true)) {
//         std::cout << "✓ Successfully saved test.wav" << std::endl;
//     } else {
//         std::cout << "✗ Failed to save test.wav" << std::endl;
//         return;
//     }

//     // 3. Проверяем WAV файл
//     std::cout << "\n3. Validating test.wav..." << std::endl;
//     WAVInfo info = AudioHandler::validateWAV("test.wav", true);
//     if (!info.errorMsg.empty()) {
//         std::cout << "✗ Validation failed: " << info.errorMsg << std::endl;
//         return;
//     }
//     std::cout << "✓ Validation passed" << std::endl;

//     // 4. Читаем обратно с отладкой
//     std::cout << "\n4. Reading back from test.wav with debug output..." << std::endl;
//     std::vector<short> readSamples;
//     int readSampleRate;
//     if (AudioHandler::readWAV("test.wav", readSamples, readSampleRate, true)) {
//         std::cout << "✓ Successfully read test.wav" << std::endl;
//         printAudioStats(readSamples, "Read Signal");
//     } else {
//         std::cout << "✗ Failed to read test.wav" << std::endl;
//         return;
//     }

//     // 5. Сравниваем
//     std::cout << "\n5. Comparing original and read signals..." << std::endl;
//     if (originalSamples.size() == readSamples.size()) {
//         bool identical = true;
//         int diffCount = 0;
//         int maxDiff = 0;

//         for (size_t i = 0; i < originalSamples.size(); ++i) {
//             int diff = std::abs(originalSamples[i] - readSamples[i]);
//             if (diff > 0) {
//                 identical = false;
//                 diffCount++;
//                 maxDiff = std::max(maxDiff, diff);
//             }
//         }

//         if (identical) {
//             std::cout << "✓ Perfect match! Files are identical." << std::endl;
//         } else {
//             std::cout << "⚠ Samples differ in " << diffCount << " positions." << std::endl;
//             std::cout << "  Maximum difference: " << maxDiff << std::endl;
//         }
//     } else {
//         std::cout << "✗ Size mismatch: " << originalSamples.size()
//                   << " vs " << readSamples.size() << std::endl;
//     }

//     // 6. Дополнительный тест: читаем WAV и сразу перезаписываем
//     std::cout << "\n6. Testing copy operation (read -> write)..." << std::endl;
//     if (AudioHandler::readWAV("test.wav", readSamples, readSampleRate, false)) {
//         if (AudioHandler::writeWAV("test_copy.wav", readSamples, readSampleRate, true)) {
//             std::cout << "✓ Copy operation successful" << std::endl;
//             AudioHandler::compareWAVs("test.wav", "test_copy.wav", 0.0);
//         }
//     }

//     // 7. Сравнительная проверка с помощью внешней функции
//     std::cout << "\n7. Final comparison using compareWAVs function..." << std::endl;
//     AudioHandler::compareWAVs("test.wav", "test_copy.wav", 0.1);

//     std::cout << "\n=== Test Complete ===" << std::endl;
// }

// // Тест с реальным WAV файлом (если есть)
// void testWithRealWAV(const std::string& filename) {
//     std::cout << "\n=== Testing with real WAV file: " << filename << " ===" << std::endl;

//     // Проверяем файл
//     WAVInfo info = AudioHandler::validateWAV(filename, true);
//     if (!info.errorMsg.empty()) {
//         std::cout << "Failed to validate: " << info.errorMsg << std::endl;
//         return;
//     }

//     // Читаем файл
//     std::vector<short> samples;
//     int sampleRate;
//     if (AudioHandler::readWAV(filename, samples, sampleRate, true)) {
//         printAudioStats(samples, "Original File");

//         // Сохраняем копию
//         std::string outputFilename = "copy_of_" + filename;
//         if (AudioHandler::writeWAV(outputFilename, samples, sampleRate, true)) {
//             // Сравниваем
//             AudioHandler::compareWAVs(filename, outputFilename, 0.0);
//         }
//     }
// }

// int main(int argc, char* argv[]) {
//     std::cout << "AudioHandler Test Suite" << std::endl;
//     std::cout << "=======================" << std::endl;

//     // Основной тест
//     testWAVReadWrite();

//     // Если передан аргумент командной строки, тестируем его
//     if (argc > 1) {
//         testWithRealWAV(argv[1]);
//     }

//     return 0;
// }
