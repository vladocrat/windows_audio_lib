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

#include <QObject>

#include <audioclient.h>
#include <string>
#include <wrl/client.h>
#include <mmdeviceapi.h>

#include <slk/general.h>
#include <slk/audiobuffer.h>

struct IMMDevice;

namespace slk {

using Microsoft::WRL::ComPtr;

class DeviceManager;

struct DeviceInfo
{
    std::wstring friendlyName;
    slk::DeviceType type;
    ComPtr<IMMDevice> device;


    ~DeviceInfo()
    {

    }

    DeviceInfo(DeviceInfo&& other) = default;
    DeviceInfo() = default;
    DeviceInfo& operator=(DeviceInfo&& other) = delete;
    DeviceInfo(const DeviceInfo&) = delete;
    DeviceInfo& operator=(const DeviceInfo&) = delete;
};

class Device : public QObject
{
    Q_OBJECT
public:
    virtual ~Device();

    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    
    friend class DeviceManager;
    
signals:
    void readyRead(const AudioBuffer<float>&);
};

} //! slk

