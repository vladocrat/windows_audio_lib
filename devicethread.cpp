#include "devicethread.h"

#include <QDebug>

namespace slk
{

struct DeviceThread::impl_t
{
    IAudioCaptureClient* captureClient;
    HANDLE hEvent;
    UINT32 bufferFrameSize;
    WAVEFORMATEX* format;

    impl_t(IAudioCaptureClient* captureClient, HANDLE hEvent, UINT32 bufferFrameSize, WAVEFORMATEX* format)
        : captureClient(captureClient)
        , hEvent(hEvent)
        , bufferFrameSize(bufferFrameSize)
        , format(format)
    {

    }
};

DeviceThread::DeviceThread(IAudioCaptureClient* captureClient, HANDLE hEvent, UINT32 bufferFrameSize, WAVEFORMATEX* format)
{
    createImpl(captureClient, hEvent, bufferFrameSize, format);

}

DeviceThread::~DeviceThread()
{

}

void DeviceThread::run()
{
    while (!isInterruptionRequested())
    {
        DWORD waitResult = WaitForSingleObject(impl().hEvent, INFINITE);
        if (waitResult == WAIT_OBJECT_0)
        {
            BYTE* pData = { nullptr };
            UINT32 numFrames = 0;
            DWORD flags = 0;
            HRESULT hr = impl().captureClient->GetBuffer(&pData, &numFrames, &flags, NULL, NULL);
            if (FAILED(hr))
            {
                qWarning() << "Failed to get buffer from capture client:" << hr;
                break;
            }

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                pData = nullptr;
            }

            if (numFrames > 0)
            {
                emit audioDataReady(pData, numFrames, flags);
            }

            impl().captureClient->ReleaseBuffer(numFrames);
        }
        else
        {
            qWarning() << "WaitForSingleObject failed or timed out";
            break;
        }
    }
}

}
