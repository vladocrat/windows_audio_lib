#include "recordingdevice.h"

#include <audiopolicy.h>
#include <mmdeviceapi.h>

struct RecordingDevice::impl_t
{
    uint32_t bufferFrameSize { 0 };
    DWORD statusData { 0 };
    BYTE* data { nullptr };
    IAudioCaptureClient* client { nullptr };
};

RecordingDevice::RecordingDevice()
{
    createImpl();
}

RecordingDevice::~RecordingDevice()
{
    delete[] impl().data;
    impl().client->Release();
}

bool RecordingDevice::initialize() noexcept
{
    auto err = client()->GetService(__uuidof(IAudioCaptureClient),
                                    reinterpret_cast<void**>(&impl().client));

    if (FAILED(err)) return false;

    err = client()->Start();

    if (FAILED(err)) return false;

    err = client()->GetBufferSize(&impl().bufferFrameSize);

    if (FAILED(err)) return false;

    return SUCCEEDED(err);
}

bool RecordingDevice::record() noexcept
{
    auto err = impl().client->GetBuffer(&impl().data,
                                        &impl().bufferFrameSize,
                                        &impl().statusData,
                                        NULL,
                                        NULL);

    return SUCCEEDED(err);
}

bool RecordingDevice::play() noexcept
{
    auto err = impl().client->ReleaseBuffer(impl().bufferFrameSize);

    return SUCCEEDED(err);
}
