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

#include <slk/audiofile.h>
#include <slk/audioformat.h>

namespace slk
{

File::File(const std::string& name, const Access access)
{
    open(name, access);
}

File::~File()
{
    close();
}

bool File::open(const std::string& name, const Access access)
{
    switch (access)
    {
    case Access::Read:
        _file.open(name, std::ios::in | std::ios::binary);
        break;
    case Access::Write:
        _file.open(name, std::ios::out | std::ios::binary);
        break;
    case Access::ReadWrite:
        _file.open(name, std::ios::in | std::ios::out | std::ios::binary);
        break;
    }

    return _file.is_open();
}

bool File::close()
{
    _file.close();
    return !_file.is_open();
}

bool File::isOpen() const
{
    return _file.is_open();
}

void File::read(char* dest, size_t size)
{
    _file.read(dest, size);
}

void File::read(std::span<char> dest, size_t size)
{
    _file.read(dest.data(), size);
}

void File::skip(size_t bytes)
{
    _file.seekg(bytes, std::ios::cur);
}

WAV::WAV(const std::string& name, const Access access)
    : _file(name, access)
{

}

void WAV::write(const AudioBuffer<float>& data)
{

}

AudioFormat WAV::format() const
{
    return AudioFormat(_header.numChannels, _header.sampleRateHz, _header.bitsPerSample, _header.audioFormat == 1 ? AudioFormat::Type::PCM : AudioFormat::Type::FLOAT);
}

const WAV::Header &WAV::header() const
{
    return _header;
}

const AudioBuffer<char>& WAV::payload() const
{
    return _data;
}

bool WAV::isOpen() const
{
    return _file.isOpen();
}

}
