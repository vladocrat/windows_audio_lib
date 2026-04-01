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

#include <slk/audiobuffer.h>
#include <fstream>
#include <span>

namespace slk
{

enum class Access
{
    Read,
    Write,
    ReadWrite
};

class File
{
public:
    File(const std::string& name, const Access access);
    ~File();

    bool open(const std::string& name, const Access access);
    bool close();
    bool isOpen() const;
    void read(char* dest, size_t size);
    void read(std::span<char> dest, size_t size);
    void skip(size_t bytes);

private:
    std::fstream _file;
};

class WAV
{
public:
    struct Header
    {
        uint32_t fileTypeBlocID;
        uint32_t fileSize;
        uint32_t fileFormatID;

        uint32_t formatBlocID;
        uint32_t blocSize;
        uint16_t audioFormat;
        uint16_t numChannels;
        uint32_t sampleRateHz;
        uint32_t bytesPerSec; //! sampleRate * bytesPerBlock
        uint16_t bytesPerBlock; //! numChannels * bitsPerSample / 8
        uint16_t bitsPerSample;

        uint32_t dataBlocID;
        uint32_t dataSize;
    };

    WAV(const std::string& name, const Access access);

    void write(const AudioBuffer<float>& data);

    bool read()
    {
        Header header {};
        readVal(&header.fileTypeBlocID);
        readVal(&header.fileSize);
        readVal(&header.fileFormatID);

        bool foundFmt = false;
        bool foundData = false;

        while (!foundFmt || !foundData) {
            uint32_t chunkID {}, chunkSize {};
            readVal(&chunkID);
            readVal(&chunkSize);

            if (chunkID == 0x20746D66) { // "fmt "
                header.formatBlocID = chunkID;
                header.blocSize = chunkSize;
                readVal(&header.audioFormat);
                readVal(&header.numChannels);
                readVal(&header.sampleRateHz);
                readVal(&header.bytesPerSec);
                readVal(&header.bytesPerBlock);
                readVal(&header.bitsPerSample);

                if (chunkSize > 16) {
                    _file.skip(chunkSize - 16);
                }
                foundFmt = true;
            }
            else if (chunkID == 0x61746164) { // "data"
                header.dataBlocID = chunkID;
                header.dataSize = chunkSize;
                foundData = true;
            }
            else {
                _file.skip(chunkSize);
            }
        }

        _header = header;

        _data.setSize(_header.numChannels, _header.dataSize / _header.numChannels);
        _file.read(_data.data().data(), _header.dataSize);
        return true;
    }

    AudioFormat format() const;
    const Header& header() const;
    const AudioBuffer<char>& payload() const;
    bool isOpen() const;

private:
    template<class T>
    void readVal(T* val)
    {
        _file.read(reinterpret_cast<char*>(val), sizeof(T));
    }

    File _file;
    Header _header;
    AudioBuffer<char> _data;
};

}
