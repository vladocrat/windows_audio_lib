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

#include "wasapidevice.h"
#include "audiodata.h"
#include "audioformat.h"

#include <mmdeviceapi.h>

namespace
{
using namespace std::chrono_literals;

const auto BUFFER_LATENCY = 10ms;
}

namespace slk
{

struct WASAPIDevice::impl_t
{
    DeviceInfo info;
    IAudioClient* client { nullptr };
    AudioBuffer<BYTE> data;
    AudioFormat format;

    impl_t(DeviceInfo&& info)
        : info { std::move(info) }
    {

    }

    ~impl_t()
    {
        if (client) {
            client->Release();
        }

    }
};

WASAPIDevice::WASAPIDevice(DeviceInfo&& info)
{
    createImpl(std::move(info));
}

WASAPIDevice::~WASAPIDevice()
{

}

bool WASAPIDevice::open(const DWORD streamFlags)
{
    auto res = info().device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, reinterpret_cast<void**>(&impl().client));

    if (res != S_OK) {
        return false;
    }

    impl().format.setFormat(impl().client);

    res = impl().client->Initialize(AUDCLNT_SHAREMODE_SHARED, streamFlags, BUFFER_LATENCY.count(), 0, impl().format.format(), NULL);

    if (res != S_OK) {
        return false;
    }

    return true;
}

const DeviceInfo& WASAPIDevice::info() const
{
    return impl().info;
}

IAudioClient* const WASAPIDevice::audioClient() const
{
    return impl().client;
}

const AudioBuffer<BYTE>& WASAPIDevice::buffer() const
{
    return impl().data;
}

const AudioFormat& WASAPIDevice::format() const
{
    return impl().format;
}

}
