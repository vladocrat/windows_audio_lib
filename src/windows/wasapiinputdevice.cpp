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

#include "wasapiinputdevice.h"

#include <Windows.h>
#include <cstring>

#include "wasapidevice.h"

#include <slk/audioformat.h>

namespace slk
{

struct WASAPIInputDevice::impl_t // NOLINT(cppcoreguidelines-special-member-functions)
{
    WASAPIDevice device;
    IAudioCaptureClient* client { nullptr };
    std::atomic_bool shouldStop { false };
    // Auto-reset event signalled by WASAPI when capture data is ready.
    HANDLE deviceEvent { nullptr };
    // Manual-reset event signalled by stop(); the capture loop waits on
    // this alongside deviceEvent so it can wake without stop() ever
    // closing a handle that the worker thread is mid-Wait* on.
    HANDLE stopEvent { nullptr };

    WASAPIInputDevice::ProcessCallback processCallback {};

    impl_t(DeviceInfo&& info) : device(std::move(info))
    {
    }

    ~impl_t()
    {
        if (client) {
            client->Release();
        }

        if (deviceEvent) {
            CloseHandle(deviceEvent);
        }

        if (stopEvent) {
            CloseHandle(stopEvent);
        }
    }
};

WASAPIInputDevice::WASAPIInputDevice(DeviceInfo&& info)
{
    createImpl(std::move(info));
}

WASAPIInputDevice::~WASAPIInputDevice() = default;

bool WASAPIInputDevice::open()
{
    const auto res = impl().device.open(AUDCLNT_STREAMFLAGS_EVENTCALLBACK);

    if (!res) {
        return false;
    }

    const auto hr = impl().device.audioClient()->GetService(__uuidof(IAudioCaptureClient),
                                                            reinterpret_cast<void**>(&impl().client));

    return hr == S_OK;
}

bool WASAPIInputDevice::close()
{
    if (!impl().client) {
        return true;
    }

    impl().client->Release();
    impl().client = nullptr;

    return true;
}

bool WASAPIInputDevice::start()
{
    impl().shouldStop = false;

    // Auto-reset for the WASAPI data-ready notification, manual-reset for
    // the stop signal so a single SetEvent reliably wakes (and stays
    // observed by) the loop on the next WaitForMultipleObjects iteration.
    impl().deviceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    impl().stopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (!impl().deviceEvent || !impl().stopEvent) {
        return false;
    }

    const auto res = impl().device.audioClient()->SetEventHandle(impl().deviceEvent);

    if (res != S_OK) {
        return false;
    }

    if (impl().device.audioClient()->Start() != S_OK) {
        return false;
    }

    const auto channels = impl().device.format().channels();

    HANDLE waitHandles[2] = { impl().deviceEvent, impl().stopEvent };

    while (!impl().shouldStop) {
        const auto result = WaitForMultipleObjects(2, waitHandles, FALSE, 2000);

        if (result == WAIT_TIMEOUT) {
            continue;
        }

        // Stop event fired — exit cleanly. The handle is owned by impl_t
        // and only released after the loop returns, so we never wait on a
        // handle that another thread is about to close.
        if (result == WAIT_OBJECT_0 + 1) {
            break;
        }

        if (result != WAIT_OBJECT_0) {
            break;
        }

        UINT32 packetLength = 0;
        auto hr = impl().client->GetNextPacketSize(&packetLength);

        while (SUCCEEDED(hr) && packetLength > 0 && !impl().shouldStop) {
            BYTE* data { nullptr };
            UINT32 numFrames { 0 };
            DWORD flags { 0 };

            hr = impl().client->GetBuffer(&data, &numFrames, &flags, nullptr, nullptr);

            if (FAILED(hr)) {
                continue;
            }

            if (numFrames > 0 && !(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                AudioBuffer<float> captureBuffer(channels, numFrames);

                const size_t samplesToCopy = static_cast<size_t>(numFrames) * channels;
                std::memcpy(captureBuffer.data().data(), data, samplesToCopy * sizeof(float));

                if (impl().processCallback) {
                    impl().processCallback(captureBuffer);
                }
            }

            impl().client->ReleaseBuffer(numFrames);

            hr = impl().client->GetNextPacketSize(&packetLength);
        }
    }

    // Loop has exited — safe to release the WASAPI clock and the wait
    // handles. Doing this here (rather than in stop()) guarantees no
    // other thread is inside WaitForMultipleObjects on these handles.
    impl().device.audioClient()->Stop();

    if (impl().deviceEvent) {
        CloseHandle(impl().deviceEvent);
        impl().deviceEvent = nullptr;
    }

    if (impl().stopEvent) {
        CloseHandle(impl().stopEvent);
        impl().stopEvent = nullptr;
    }

    return true;
}

bool WASAPIInputDevice::stop()
{
    // Signal the capture loop to exit. The loop owns the handle lifetime
    // and will close both events itself before start() returns. Callers
    // are still expected to join the worker thread (e.g. via the future
    // tracking start()) before destroying or reopening the device.
    impl().shouldStop = true;

    if (impl().stopEvent) {
        SetEvent(impl().stopEvent);
    }

    return true;
}

void WASAPIInputDevice::setProcessCallback(ProcessCallback callback)
{
    impl().processCallback = std::move(callback);
}

const AudioFormat& WASAPIInputDevice::format() const
{
    return impl().device.format();
}

DeviceDescriptor WASAPIInputDevice::descriptor() const
{
    return impl().device.descriptor();
}

}
