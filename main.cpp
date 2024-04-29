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

#define _USE_MATH_DEFINES
#include <math.h>

float phaseSensitiveAverage(float left, float right) {
    // Calculate the phase difference between the left and right channels
    float phaseDifference = std::atan2(right, left);

    // Normalize the phase difference to the range [0, 1]
    float normalizedPhaseDifference = (phaseDifference + M_PI) / (2.0f * M_PI);

    // Use the normalized phase difference to weight the averaging
    // This is a simple example; more sophisticated methods might use different weighting schemes
    return (left + right) * normalizedPhaseDifference;
}

int32_t compressSample(int32_t sample, float threshold, float ratio) {
    float sampleF = static_cast<float>(sample) / INT32_MAX;
    if (sampleF > threshold) {
        sampleF = threshold + (sampleF - threshold) / ratio;
    }
    return static_cast<int32_t>(sampleF * INT32_MAX);
}


int main()
{
    auto hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        std::cerr << "failed to initialize COM library: " << hr << std::endl;
        return hr;
    }

    IMMDeviceEnumerator* enumerator { nullptr };

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
                            NULL,
                            CLSCTX_ALL,
                            __uuidof(IMMDeviceEnumerator),
                            reinterpret_cast<void**>(&enumerator));

    if (FAILED(hr)) {
        std::cerr << "Failed to init enumerator: " << hr << std::endl;
        return hr;
    }

    RecordingDevice* rd = new RecordingDevice;
    auto d = rd->getdevice();

    enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &d);

    rd->initialize();


    IMMDevice* outputDevice { nullptr };

    hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &outputDevice);

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

    while (true) {
        rd->record();

        renderClient->GetBuffer(rd->frameSize(), &outputBuffer);

        CopyMemory(outputBuffer, rd->data(), rd->frameSize() * rd->waveFormat()->nBlockAlign);

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

        renderClient->ReleaseBuffer(rd->frameSize(), NULL);
        rd->play();
    }

    enumerator->Release();

    return 0;
}
