#ifndef AUDIO_HANDLER_H
#define AUDIO_HANDLER_H

#include <string>
#include <vector>

class AudioHandler {
public:
    static bool readWAV(const std::string& filename,
                        std::vector<short>& samples,
                        int& sampleRate);

    static bool writeWAV(const std::string& filename,
                         const std::vector<short>& samples,
                         int sampleRate);
};

#endif
