#include <iostream>
#include <Windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <conio.h>
#include <comdef.h>

#include "recordingdevice.h"
#include "deviceexplorer.h"

int main()
{
    auto hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        std::cerr << "failed to initialize COM library: " << hr << std::endl;
        return hr;
    }

    DeviceExplorer* explorer = new DeviceExplorer;

    RecordingDevice* rd = new RecordingDevice;

    auto d = explorer->defaultDevice(DeviceExplorer::DeviceType::Record);
    rd->setDevice(d);

    if (!rd->initialize()) {
        std::cerr << "failed to init";
    }

    IMMDevice* outputDevice { nullptr };

    outputDevice = explorer->defaultDevice(DeviceExplorer::DeviceType::Playback);

    if (FAILED(hr)) {
        std::cerr << "Failed to get outputDevice: " << hr << std::endl;
        return hr;
    }

    IAudioClient* outputAudioClient { nullptr };


    hr = outputDevice->Activate(__uuidof(IAudioClient),
                          CLSCTX_ALL,
                          NULL,
                          reinterpret_cast<void**>(&outputAudioClient));

    if (FAILED(hr)) {
        std::cerr << "Failed to activate output device: " << hr << std::endl;
        return hr;
    }

    hr = outputAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, rd->waveFormat(), NULL);

    if (FAILED(hr)) {
        std::cerr << "Failed to initialize outputAudioClient: " << hr << std::endl;
        return hr;
    }

    IAudioRenderClient* renderClient { nullptr };

    hr = outputAudioClient->GetService(__uuidof(IAudioRenderClient),
                                       reinterpret_cast<void**>(&renderClient));

    if (FAILED(hr)) {
        std::cerr << "Failed to initialize render client: " << hr << std::endl;
        return hr;
    }

    BYTE* outputBuffer { nullptr };
    DWORD statusData;

    outputAudioClient->Start();

    outputBuffer = new BYTE[rd->frameSize() * rd->waveFormat()->nBlockAlign];
    std::cout << "starting writing";

    while (true) {
        if (!rd->record()) {
            std::cerr << "failed to record" << std::endl;
        }

        renderClient->GetBuffer(rd->frameSize(), &outputBuffer);

        CopyMemory(outputBuffer, rd->data(), rd->frameSize() * rd->waveFormat()->nBlockAlign);

        renderClient->ReleaseBuffer(rd->frameSize(), NULL);

        if (!rd->play()) {
            std::cerr << "failed to play" << std::endl;
        }
    }

    return 0;
}

// if (waveFormat->nChannels != 2) {
//     for (DWORD i = 0; i < bufferFrameCount; i+=2) {
//         auto mono = (int32_t*)data + i;
//         auto left = (int32_t*)outputBuffer + i;
//         auto right = (int32_t*)outputBuffer + i + 1;

//         auto sample = compressSample(*mono, 1000.0f, .01f);

//         *left = sample;
//         *right = sample;
//     }
// } else {
//     CopyMemory(outputBuffer, data, bufferFrameCount * waveFormat->nBlockAlign);
// }

