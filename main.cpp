#include <iostream>
#include <Windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <conio.h>
#include <comdef.h>

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

    IAudioClient* audioClient { nullptr };
    IMMDevice* device { nullptr };

    hr = enumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &device);

    if (FAILED(hr)) {
        std::cerr << "Failed to get default audio capture device: " << hr << std::endl;
        return hr;
    }

    hr = device->Activate(__uuidof(IAudioClient),
                          CLSCTX_ALL,
                          NULL,
                          reinterpret_cast<void**>(&audioClient));

    if (FAILED(hr)) {
        std::cerr << "Failed to activate device: " << hr << std::endl;
        return hr;
    }

    WAVEFORMATEX* waveFormat { nullptr };

    hr = audioClient->GetMixFormat(&waveFormat);

    if (FAILED(hr)) {
        std::cerr << "Failed to get mix format: " << hr << std::endl;
        return hr;
    }

    hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, waveFormat, NULL);

    if (FAILED(hr)) {
        std::cerr << "Failed to initialize client: " << hr << std::endl;
        return hr;
    }

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

    hr = outputAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, 0, 0, waveFormat, NULL);

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

    uint32_t bufferFrameCount { 0 };

    IAudioCaptureClient* captureClient { nullptr };


    audioClient->GetService(__uuidof(IAudioCaptureClient), reinterpret_cast<void**>(&captureClient));

    BYTE* data { nullptr };
    BYTE* outputBuffer { nullptr };
    DWORD statusData;

    audioClient->Start();
    outputAudioClient->Start();

    audioClient->GetBufferSize(&bufferFrameCount);

    data = new BYTE[bufferFrameCount * waveFormat->nBlockAlign];
    outputBuffer = new BYTE[bufferFrameCount * waveFormat->nBlockAlign];

    while (true) {
        captureClient->GetBuffer(&data, &bufferFrameCount, &statusData, NULL, NULL);

        renderClient->GetBuffer(bufferFrameCount, &outputBuffer);

        if (waveFormat->nChannels != 2) {
            for (DWORD i = 0; i < bufferFrameCount; i+=2) {
                auto mono = (int32_t*)data + i;
                auto left = (int32_t*)outputBuffer + i;
                auto right = (int32_t*)outputBuffer + i + 1;

                auto sample = compressSample(*mono, 1000.0f, .01f);

                *left = sample;
                *right = sample;
            }
        } else {
            CopyMemory(outputBuffer, data, bufferFrameCount * waveFormat->nBlockAlign);
        }

        renderClient->ReleaseBuffer(bufferFrameCount, NULL);
        captureClient->ReleaseBuffer(bufferFrameCount);
    }

    enumerator->Release();
    audioClient->Stop();
    audioClient->Release();
    device->Release();

    return 0;
}
