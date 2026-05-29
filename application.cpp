#include "application.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

int printUsage() {
    std::cout << "Usage:\n";
    std::cout << "  isaccodec encode 16000 <input.wav> <output.isac>\n";
    std::cout << "  isaccodec decode 16000 <input.isac> <output.wav>\n";
    return 1;
}

int Application::exec(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Should be 5 args\n";
        return printUsage();
    }

    std::string mode = argv[1];
    int sampleRate = std::atoi(argv[2]);
    std::string inputFile = argv[3];
    std::string outputFile = argv[4];

    // std::cout << sampleRate << std::endl;

    // std::string mode = "encode";
    // int sampleRate = 16000;
    // std::string inputFile = "C:\\Qt\\Projects\\isac4\\build\\Desktop_Qt_6_7_2_MinGW_64_bit-Debug\\isac_app\\audio.wav";
    // std::string outputFile = "C:\\Qt\\Projects\\isac4\\build\\Desktop_Qt_6_7_2_MinGW_64_bit-Debug\\isac_app\\coded.isac";


    ISACCodec codec(sampleRate);

    try {
        if (mode == "encode") {
            std::vector<short> samples;
            if (!AudioHandler::readWAV(inputFile, samples, sampleRate)) {
                return 1;
            }

            const int samples10ms = sampleRate / 100;
            std::ofstream outFile(outputFile, std::ios::binary);
            std::vector<unsigned char> encodedPacket;

            int frameCount = 0;
            for (size_t i = 0; i < samples.size(); i += samples10ms) {
                if (i + samples10ms > samples.size()) break;

                std::vector<short> frame10ms(samples.begin() + i,
                                             samples.begin() + i + samples10ms);

                // Передаем 10ms фрейм в кодек
                if (codec.encode(frame10ms, encodedPacket)) {
                    // ISAC закодировал полный 30ms фрейм
                    uint16_t packetSize = static_cast<uint16_t>(encodedPacket.size());
                    outFile.write(reinterpret_cast<const char*>(&packetSize), sizeof(packetSize));
                    outFile.write(reinterpret_cast<const char*>(encodedPacket.data()), packetSize);
                    // std::cout << "Frame " << ++frameCount << ": wrote " << packetSize << " bytes\n";
                }
                // Если encode вернул false, продолжать
            }

            outFile.close();
            std::cout << "Encoded to " << outputFile << "\n";
        } /*else if (mode == "decode") {
            std::ifstream inFile(inputFile, std::ios::binary);
            if (!inFile.is_open()) {
                std::cerr << "Cannot read: " << inputFile << "\n";
                return 1;
            }

            std::vector<short> allPcm;
            const int frameLen = (sampleRate == 16000) ? 480 : 960;
            std::vector<unsigned char> packet;
            packet.resize(1000);

            while (inFile.read(reinterpret_cast<char*>(packet.data()), 10)) {
                int packetLen = static_cast<int>(inFile.gcount());
                packet.resize(packetLen);
                std::vector<unsigned char> fullPacket(packet);
                std::vector<short> decodedFrame;
                if (codec.decode(fullPacket, decodedFrame)) {
                    allPcm.insert(allPcm.end(), decodedFrame.begin(), decodedFrame.end());
                }
            }

            if (!AudioHandler::writeWAV(outputFile, allPcm, sampleRate)) {
                return 1;
            }
            std::cout << "Decoded to " << outputFile << "\n";

        } else {
            return printUsage();
        }*/

        else if (mode == "decode") {
            std::ifstream inFile(inputFile, std::ios::binary);
            if (!inFile.is_open()) {
                std::cerr << "Cannot read: " << inputFile << "\n";
                return 1;
            }

            std::vector<short> allPcm;

            // Читаем фреймы с префиксом размера
            while (inFile.peek() != EOF) {
                uint16_t packetSize;
                inFile.read(reinterpret_cast<char*>(&packetSize), sizeof(packetSize));
                if (inFile.gcount() != sizeof(packetSize)) break;

                std::vector<unsigned char> packet(packetSize);
                inFile.read(reinterpret_cast<char*>(packet.data()), packetSize);
                if (inFile.gcount() != packetSize) break;

                std::vector<short> decodedFrame;
                if (codec.decode(packet, decodedFrame)) {
                    // decodedFrame.size() должно быть 480 (30ms) или 960 (60ms)
                    // std::cout << "Got " << decodedFrame.size() << " decoded samples\n";
                    allPcm.insert(allPcm.end(), decodedFrame.begin(), decodedFrame.end());
                } else {
                    std::cerr << "Warning: Failed to decode frame\n";
                    // Вставляем тишину длительностью 30ms
                    int samples30ms = sampleRate * 30 / 1000;
                    decodedFrame.assign(samples30ms, 0);
                    allPcm.insert(allPcm.end(), decodedFrame.begin(), decodedFrame.end());
                }
            }

            inFile.close();

            // Проверка: количество семплов должно быть кратно 480 (для 30ms фреймов)
            // std::cout << "Total decoded samples: " << allPcm.size()
            //           << " (" << (allPcm.size() * 1000.0 / sampleRate) << " ms)\n";

            if (!AudioHandler::writeWAV(outputFile, allPcm, sampleRate)) {
                return 1;
            }
            std::cout << "Decoded to " << outputFile << "\n";
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
