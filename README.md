# windows_audio_lib

A C++20 Windows audio library built on WASAPI and Qt6. It provides device enumeration, low-latency audio capture and playback, WAV file I/O, and a suite of DSP tools (DFT, filtering, windowing, noise generation) under the `slk` namespace.

---

## Prerequisites

| Requirement | Version | Notes |
|---|---|---|
| **Windows** | 10 or later | WASAPI is required |
| **C++ compiler** | MSVC 2019+ or Clang/LLVM with MSVC runtime | C++20 support required |
| **CMake** | 3.5+ | Build system |
| **Qt6** | 6.4+ | `Qt6::Core` component only |
| **Windows SDK** | 10.0+ | For `mmdeviceapi.h`, `audioclient.h`, WRL COM headers |

Qt6 can be installed via the [Qt Online Installer](https://www.qt.io/download) or vcpkg:
```
vcpkg install qt6-base
```

---

## What the library provides

- **Device management** — enumerate, select, and create WASAPI audio devices
- **Audio capture** — `WASAPIInputDevice` with an optional real-time `ProcessCallback`
- **Audio playback** — `WASAPIOutputDevice` fed from a lock-free `RingBuffer<float>`
- **WAV file I/O** — read/write WAV files with automatic format handling
- **DSP** — DFT, low-pass filter, window functions (Hann, FlatTop), white noise generator
- **Audio buffers** — multi-channel `AudioBuffer<T>`, lock-free `RingBuffer<T>`, functional filter piping (`buffer | filter`)

---

## Integrating into a CMake project

### Step 1 — add the library

**Option A: subdirectory (source already on disk)**
```cmake
add_subdirectory(path/to/windows_audio_lib)
```

**Option B: FetchContent**
```cmake
include(FetchContent)
FetchContent_Declare(
    windows_audio_lib
    GIT_REPOSITORY https://github.com/vladocrat/windows_audio_lib.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(windows_audio_lib)
```

### Step 2 — link against the target

```cmake
target_link_libraries(your_target PRIVATE sound_capture)
```

### Step 3 — make Qt6 discoverable

If Qt is not on the system path, point CMake to it before the first `find_package` call:
```
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.x.x/msvc2019_64" ..
```

### Minimal CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.5)
project(my_app LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(windows_audio_lib)

add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE sound_capture)
```

---

## Usage

### Input device (recording)

```cpp
#include "devicemanager.h"
#include "wasapiinputdevice.h"
#include "filter.h"

using namespace slk;

DeviceManager manager;
auto device = manager.defaultDevice(DeviceType::Record, Purpose::Multimedia);
auto input  = std::dynamic_pointer_cast<WASAPIInputDevice>(device);

// Optional: process each buffer in-place before readyRead fires
filter::LowPassFilter<float> lpf(5000.0f, input->format().sampleRate());
input->setProcessCallback([&](AudioBuffer<float>& buf) {
    buf | lpf;
});

// Receive processed buffers via Qt signal
QObject::connect(input.get(), &Device::readyRead,
    [](const AudioBuffer<float>& buf) {
        // buf contains interleaved float samples ready for further use
    });

input->open();
input->start();

// ... run Qt event loop ...

input->stop();
input->close();
```

### Output device (playback)

```cpp
#include "devicemanager.h"
#include "wasapioutputdevice.h"
#include "ringbuffer.h"

using namespace slk;

RingBuffer<float> ring(4096);

DeviceManager manager;
auto device = manager.defaultDevice(DeviceType::Playback, Purpose::Multimedia);
auto output = std::dynamic_pointer_cast<WASAPIOutputDevice>(device);
output->setSource(&ring);

output->open();
output->start();

// Producer thread writes samples into the ring buffer:
//   ring.write(samples.data(), samples.size());

// ... run Qt event loop ...

output->stop();
output->close();
```

### DSP: compute a frequency spectrum

```cpp
#include "dsp.h"
#include "window.h"

using namespace slk;

Window<Hann, float, 1024> win;
win.apply(buffer);                                        // apply Hann window in-place

auto spectrum = dsp::dft(buffer);                         // compute DFT
auto freqMags = dsp::freqMag(spectrum, buffer.sampleRate()); // map to Hz / magnitude pairs
```

---

## License

This library is distributed under the **GNU General Public License v3.0 or later**. See [LICENSE](LICENSE) for the full text.
