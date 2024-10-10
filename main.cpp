#include <QCoreApplication>

#include <Windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <audiopolicy.h>
#include <conio.h>
#include <comdef.h>

#include <iostream>

#include "deviceexplorer.h"
#include "devicemanager.h"
#include "device.h"

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    auto hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(hr)) {
        std::cerr << "failed to initialize COM library: " << hr << std::endl;
        return hr;
    }
    
    slk::DeviceManager manager;
    const auto rd = manager.create(slk::DeviceType::Record);
    const auto pd = manager.create(slk::DeviceType::Playback);
    
    QObject::connect(rd, &slk::Device::readyRead, [=](const slk::Device::Data& data) {
        pd->playback(data);
    });
    
    rd->start();
    pd->start();
        
    return app.exec();
}
