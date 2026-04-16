# windows_audio_lib

A C++20 Windows audio library built on WASAPI. It provides device enumeration, low-latency audio capture and playback, WAV file I/O, and a suite of DSP tools (DFT, filtering, windowing, noise generation) under the `slk` namespace.

---

## Prerequisites

| Requirement | Version | Notes |
|---|---|---|
| **Windows** | 10 or later | WASAPI is required |
| **C++ compiler** | MSVC 2019+ or Clang/LLVM with MSVC runtime | C++20 support required |
| **CMake** | 3.5+ | Build system |
| **Windows SDK** | 10.0+ | Required for WASAPI backend |

---

## What the library provides

- **Device management** — enumerate and select audio devices via platform-agnostic `DeviceDescriptor`
- **Audio capture** — `InputDevice` with an optional real-time `ProcessCallback`
- **Audio playback** — `OutputDevice` fed from a lock-free `RingBuffer<float>`
- **WAV file I/O** — read/write WAV files with automatic format handling
- **DSP** — DFT, low-pass filter, window functions (Hann, FlatTop), white noise generator
- **Filter chain** — compose multiple filters into a single callable via `makeChain<T>()`
- **Audio graph** — `AudioGraph<T>` for arbitrary DAG topologies with automatic buffer management and implicit mixing
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

### Enumerating devices

```cpp
#include <slk/deviceexplorer.h>

using namespace slk;

DeviceExplorer explorer;
auto inputDevices  = explorer.devices(DeviceType::Record, DeviceState::Active);
auto outputDevices = explorer.devices(DeviceType::Playback, DeviceState::Active);

for (const auto& desc : inputDevices) {
    // desc.name — friendly name (std::wstring)
    // desc.id   — opaque device identifier
    // desc.type — DeviceType::Record
}
```

### Input device (recording)

```cpp
#include <slk/sound_capture.h>
// or include individually:
// #include <slk/devicemanager.h>
// #include <slk/inputdevice.h>
// #include <slk/dsp/filter.h>

using namespace slk;

DeviceManager manager;
auto input = manager.defaultInputDevice(Purpose::Multimedia);

// Optional: process each buffer in-place
filter::LowPassFilter<float> lpf(5000.0f, input->format().sampleRate());
input->setProcessCallback([&](AudioBuffer<float>& buf) {
    buf | lpf;
});

input->open();

// start() blocks — run it on a background thread
std::thread captureThread([&]() { input->start(); });

// ... do work ...

input->stop();
captureThread.join();
input->close();
```

### Output device (playback)

```cpp
#include <slk/sound_capture.h>
// or include individually:
// #include <slk/devicemanager.h>
// #include <slk/outputdevice.h>
// #include <slk/ringbuffer.h>

using namespace slk;

RingBuffer<float> ring(4096);

DeviceManager manager;
auto output = manager.defaultOutputDevice(Purpose::Multimedia);
output->setSource(ring);

output->open();

// start() blocks — run it on a background thread
std::thread playbackThread([&]() { output->start(); });

// Producer writes samples into the ring buffer:
//   ring.write(samples.data(), samples.size());

// ... do work ...

output->stop();
playbackThread.join();
output->close();
```

### Creating a specific device

```cpp
#include <slk/deviceexplorer.h>
#include <slk/devicemanager.h>
#include <slk/inputdevice.h>

using namespace slk;

DeviceExplorer explorer;
auto devices = explorer.devices(DeviceType::Record, DeviceState::Active);

DeviceManager manager;
auto input = manager.createInputDevice(devices[0]);

input->open();
input->start();
```

### FilterChain

```cpp
#include <slk/dsp/chain.h>
#include <slk/dsp/filter.h>

using namespace slk;

filter::SimpleGainFilter<float>  gain(1.5f);
filter::LowPassFilter<float>     lpf(1000.0f, 48000.0f);
filter::SimpleSoftLimiter<float> limiter(0.9f);

auto chain = dsp::makeChain<float>(gain, lpf, limiter);

AudioBuffer<float> buf(2, 512);
chain(buf);           // applies gain → lpf → limiter in order
// or: buf | chain;   // pipe syntax
```

### AudioGraph

```cpp
#include <slk/dsp/graph.h>
#include <slk/dsp/filter.h>

using namespace slk;

filter::SimpleGainFilter<float>  gainL(0.8f);
filter::SimpleGainFilter<float>  gainR(0.6f);
filter::SimpleSoftLimiter<float> limiter(0.9f);

dsp::AudioGraph<float> graph;

auto hL   = graph.addNode(gainL);
auto hR   = graph.addNode(gainR);
auto hLim = graph.addNode(limiter);

// Both paths feed into the limiter — buffers are summed automatically
graph.connect(hL,  hLim);
graph.connect(hR,  hLim);
graph.setOutput(hLim);

graph.compile(2, 512);   // channels, samples per buffer

AudioBuffer<float> buf(2, 512);
graph.process(buf);

// Or use as a device callback:
// device->setProcessCallback(graph.asCallback());
```

### DSP: compute a frequency spectrum

```cpp
#include <slk/dsp/dsp.h>
#include <slk/dsp/window.h>

using namespace slk;

Window<Hann, float, 1024> win;
win.apply(buffer);                                        // apply Hann window in-place

auto spectrum = dsp::dft(buffer);                         // compute DFT
auto freqMags = dsp::freqMag(spectrum, buffer.sampleRate()); // map to Hz / magnitude pairs
```

---

## License

This library is distributed under the **GNU General Public License v3.0 or later**. See [LICENSE](LICENSE) for the full text.
