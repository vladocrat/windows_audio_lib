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

#include "wasapioutputdevice.h"

#include <Windows.h>

#include <cstring>

#include "wasapidevice.h"

#include <slk/audioformat.h>

namespace slk
{

struct WASAPIOutputDevice::impl_t // NOLINT(cppcoreguidelines-special-member-functions)
{
    WASAPIDevice device;
    IAudioRenderClient* client { nullptr };
    uint32_t bufferFrameCount { 0 };

    RingBuffer<float>* source { nullptr };
    WASAPIOutputDevice::ProcessCallback processCallback {};
    std::atomic_bool shouldStop { false };
    // Auto-reset event signalled by WASAPI when the render slot is ready.
    HANDLE deviceEvent { nullptr };
    // Manual-reset event signalled by stop(); the render loop waits on
    // this alongside deviceEvent so it can wake without stop() ever
    // closing a handle that the worker thread is mid-Wait* on.
    HANDLE stopEvent { nullptr };

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

WASAPIOutputDevice::WASAPIOutputDevice(DeviceInfo&& info)
{
    createImpl(std::move(info));
}

WASAPIOutputDevice::~WASAPIOutputDevice() = default;

bool WASAPIOutputDevice::open()
{
    const auto res = impl().device.open(AUDCLNT_STREAMFLAGS_EVENTCALLBACK);
    if (!res) {
        return false;
    }

    auto hr =
        impl().device.audioClient()->GetService(__uuidof(IAudioRenderClient), reinterpret_cast<void**>(&impl().client));

    if (hr != S_OK) {
        return false;
    }

    hr = impl().device.audioClient()->GetBufferSize(&impl().bufferFrameCount);

    return hr == S_OK;
}

bool WASAPIOutputDevice::close()
{
    if (!impl().client) {
        return true;
    }

    impl().client->Release();
    impl().client = nullptr;

    return true;
}

bool WASAPIOutputDevice::start()
{
    impl().shouldStop = false;

    // Auto-reset for the WASAPI render-slot notification, manual-reset for
    // the stop signal so a single SetEvent reliably wakes (and stays
    // observed by) the loop on the next WaitForMultipleObjects iteration.
    impl().deviceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    impl().stopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (!impl().deviceEvent || !impl().stopEvent) {
        return false;
    }

    auto hr = impl().device.audioClient()->SetEventHandle(impl().deviceEvent);

    if (hr != S_OK) {
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

        UINT32 numFramesPadding { 0 };
        hr = impl().device.audioClient()->GetCurrentPadding(&numFramesPadding);
        if (FAILED(hr)) {
            continue;
        }

        const auto numFramesAvailable = impl().bufferFrameCount - numFramesPadding;
        if (numFramesAvailable == 0) {
            continue;
        }

        BYTE* data { nullptr };
        hr = impl().client->GetBuffer(numFramesAvailable, &data);

        if (FAILED(hr) || !data) {
            continue;
        }

        AudioBuffer<float> tempBuffer(channels, numFramesAvailable);

        if (impl().source) {
            const size_t maxSamples = static_cast<size_t>(numFramesAvailable) * channels;
            impl().source->read(std::span<float>(tempBuffer.data().data(), maxSamples), maxSamples);
        }

        if (impl().processCallback) {
            impl().processCallback(tempBuffer);
        }

        const size_t bytesToWrite = static_cast<size_t>(numFramesAvailable) * channels * sizeof(float);
        std::memcpy(data, tempBuffer.data().data(), bytesToWrite);

        impl().client->ReleaseBuffer(numFramesAvailable, 0);
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

bool WASAPIOutputDevice::stop()
{
    // Signal the render loop to exit. The loop owns the handle lifetime
    // and will close both events itself before start() returns. Callers
    // are still expected to join the worker thread (e.g. via the future
    // tracking start()) before destroying or reopening the device.
    impl().shouldStop = true;

    if (impl().stopEvent) {
        SetEvent(impl().stopEvent);
    }

    return true;
}

void WASAPIOutputDevice::setSource(RingBuffer<float>& source)
{
    impl().source = &source;
}

void WASAPIOutputDevice::setProcessCallback(ProcessCallback callback)
{
    impl().processCallback = std::move(callback);
}

const AudioFormat& WASAPIOutputDevice::format() const
{
    return impl().device.format();
}

DeviceDescriptor WASAPIOutputDevice::descriptor() const
{
    return impl().device.descriptor();
}

}
