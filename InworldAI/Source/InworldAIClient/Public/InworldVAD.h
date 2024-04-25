/**
* Copyright 2023-2024 Theai, Inc. dba Inworld AI
 *
 * Use of this source code is governed by the Inworld.ai Software Development Kit License Agreement
 * that can be found in the LICENSE.md file or at https://www.inworld.ai/sdk-license
 */

#pragma once

#include <memory>
#include <vector>
#include <string>

namespace Inworld
{
    class VAD {

    public:
        VAD(const std::string& Model);
        ~VAD();
        
        float ProcessAudioChunk(const std::vector<float>& AudioData);
    };
}
