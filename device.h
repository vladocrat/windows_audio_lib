#pragma once

#include "general.h"
#include "utils/utils.h"

#include <audioclient.h>
#include <string>

#include <QObject>

struct IMMDevice;

namespace slk {

class DeviceManager;

struct DeviceInfo
{
    std::wstring friendlyName;
    slk::DeviceType type;
    IMMDevice* device { nullptr };
    WAVEFORMATEX* format { nullptr };
};

class Device : public QObject
{
    Q_OBJECT
public:
    struct Data
    {
        BYTE* data { nullptr };
        UINT bufferFrameSize;
        UINT size;
        DWORD status;
    };

    friend QDataStream& operator<<(QDataStream& out, const Data& data);
    friend QDataStream& operator>>(QDataStream& out, Data& data);
    
    Device();
    ~Device();

    const DeviceInfo* info() const noexcept;
    void playback(const Data&);
    void start();
    void stop();
    
    friend class DeviceManager;
    
signals:
    void readyRead(const Data&);
    
private:
    void activate() noexcept;
    void setInfo(const DeviceInfo&) noexcept;
    
private:
    DECLARE_PIMPL_EX(Device)
};

} //! slk

