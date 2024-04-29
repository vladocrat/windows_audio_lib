#include "recordingdevice.h"

#include <audiopolicy.h>
#include <mmdeviceapi.h>

struct RecordingDevice::impl_t
{
    DWORD statusData { 0 };
    IAudioCaptureClient* client { nullptr };
};

RecordingDevice::RecordingDevice()
{
    createImpl();
}

RecordingDevice::~RecordingDevice()
{
    assert(impl().client);

    impl().client->Release();
}

bool RecordingDevice::initialize() noexcept
{
    auto err = activate();

    if (FAILED(err)) return false;

    err = client()->GetService(__uuidof(IAudioCaptureClient),
                                    reinterpret_cast<void**>(&impl().client));

    if (FAILED(err)) return false;

    err = client()->Start();

    if (FAILED(err)) return false;


    err = client()->GetBufferSize(refFrameSize());

    if (FAILED(err)) return false;

    newBuffer(frameSize() * waveFormat()->nBlockAlign);

    return SUCCEEDED(err);
}

bool RecordingDevice::record() noexcept
{
    auto err = impl().client->GetBuffer(refData(),
                                        refFrameSize(),
                                        &impl().statusData,
                                        NULL,
                                        NULL);

    return SUCCEEDED(err);
}

bool RecordingDevice::play() noexcept
{
    auto err = impl().client->ReleaseBuffer(frameSize());

    return SUCCEEDED(err);
}
