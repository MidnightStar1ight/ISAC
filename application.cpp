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

    std::cout << sampleRate << std::endl;

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

            if (sampleRate != 16000 && sampleRate != 32000) {
                std::cerr << "ISAC only supports 16kHz or 32kHz. Input is " << sampleRate << "Hz\n";
                return 1;
            }

            std::vector<unsigned char> encoded;
            encoded.resize(1000);  // pre-allocate (max ISAC frame ~ 400–500 bytes)

            std::ofstream outFile(outputFile, std::ios::binary);
            if (!outFile.is_open()) {
                std::cerr << "Cannot write: " << outputFile << "\n";
                return 1;
            }

            const int frameLen = (sampleRate == 16000) ? 480 : 960;
            for (size_t i = 0; i < samples.size(); i += frameLen) {
                if (i + frameLen > samples.size()) break;

                std::vector<short> frame(samples.begin() + i, samples.begin() + i + frameLen);
                std::vector<unsigned char> packet = encoded;
                if (codec.encode(frame, packet)) {
                    outFile.write(reinterpret_cast<const char*>(packet.data()), packet.size());
                }
            }
            outFile.close();
            std::cout << "Encoded to " << outputFile << "\n";

        } else if (mode == "decode") {
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
        }

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
