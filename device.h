#pragma once

#include <QObject>

#include <audioclient.h>
#include <string>
#include <wrl/client.h>
#include <mmdeviceapi.h>

#include "general.h"
#include "audiobuffer.h"

struct IMMDevice;

namespace slk {

using Microsoft::WRL::ComPtr;

class DeviceManager;

struct DeviceInfo
{
    std::wstring friendlyName;
    slk::DeviceType type;
    ComPtr<IMMDevice> device;


    ~DeviceInfo()
    {

    }

    DeviceInfo(DeviceInfo&& other) = default;
    DeviceInfo() = default;
    DeviceInfo& operator=(DeviceInfo&& other) = delete;
    DeviceInfo(const DeviceInfo&) = delete;
    DeviceInfo& operator=(const DeviceInfo&) = delete;
};

class Device : public QObject
{
    Q_OBJECT
public:
    virtual ~Device();

    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    
    friend class DeviceManager;
    
signals:
    void readyRead(const AudioBuffer<float>&);
};

} //! slk

