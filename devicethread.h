#pragma once

#include <QThread>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

#include "utils/utils.h"

namespace slk
{

class DeviceThread : public QThread
{
    Q_OBJECT
public:
    DeviceThread(IAudioCaptureClient* captureClient, HANDLE hEvent, UINT32 bufferFrameSize, WAVEFORMATEX* format);
    ~DeviceThread();

signals:
    void audioDataReady(BYTE* data, UINT32 frames, DWORD status);

protected:
    void run() override;

private:
    DECLARE_PIMPL_EX(DeviceThread)
};

}
