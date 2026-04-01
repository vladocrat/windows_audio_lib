// windows_audio_lib - Windows audio library
// Copyright (C) 2026  Vladislav Milovanov
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "audiobuffer.h"

namespace slk
{

class AudioFormat;

template<class SampleData>
class AudioData
{
public:
    AudioData();

    uint32_t bufferSize() const;
    void setBufferSize(const uint32_t);

    AudioData<float> toFloat();

private:
    AudioBuffer<SampleData*> _data;
    uint32_t _bufferSize;
    uint64_t _status;
};

}
