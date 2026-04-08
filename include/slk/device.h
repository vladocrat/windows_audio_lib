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

#include <slk/general.h>
#include <slk/audiobuffer.h>
#include <slk/audioformat.h>

namespace slk {

class Device
{
public:
    virtual ~Device();

    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;

    virtual const AudioFormat& format() const = 0;
    virtual DeviceDescriptor descriptor() const = 0;
};

} //! slk
