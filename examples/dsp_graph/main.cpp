// windows_audio_lib - Windows audio library
// Copyright (C) 2026  Vladislav Milovanov
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// DSP graph example — demonstrates FilterChain and AudioGraph without requiring
// an audio device. Processes synthetic buffers and prints per-buffer stats.
//
// This example also shows how a GUI application would own its own graph
// descriptor and translate it into an AudioGraph — the library provides
// the graph primitives only; the translation logic lives in the application.

#include <algorithm>
#include <cassert>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <slk/audiobuffer.h>
#include <slk/dsp/chain.h>
#include <slk/dsp/filter.h>
#include <slk/dsp/graph.h>
#include <slk/dsp/noise.h>
#include <slk/types.h>

using namespace slk::dsp::literals;

// ── Application-side graph descriptor (not part of the library) ──────────────
//
// A real GUI application would have richer types here (positions, colours,
// user-facing names, etc.). This stripped-down version shows the minimum
// needed to drive AudioGraph<T>.

enum class AppNodeType
{
    LowPass,
    Gain,
    SoftLimiter
};

struct AppNodeDesc
{
    size_t                                 id;
    AppNodeType                            type;
    std::unordered_map<std::string, float> params;
};

struct AppEdgeDesc
{
    size_t srcId;
    size_t dstId;
};

struct AppGraphDesc
{
    std::vector<AppNodeDesc> nodes;
    std::vector<AppEdgeDesc> edges;
    size_t                   outputNodeId;
};

// The application owns the filter objects and maps its descriptor to library calls.
// The library has no knowledge of AppNodeType — it only sees addNode.
slk::dsp::AudioGraph<float> buildGraph(const AppGraphDesc& desc, uint32_t channels, uint32_t numSamples)
{
    slk::dsp::AudioGraph<float> graph;
    std::unordered_map<size_t, slk::dsp::AudioGraph<float>::NodeHandle> handleMap;

    for (const auto& nd : desc.nodes) {
        std::optional<slk::dsp::AudioGraph<float>::NodeHandle> h;

        switch (nd.type) {
        case AppNodeType::LowPass:
            h = graph.addNode(slk::filter::LowPassFilter<float> {
                slk::dsp::Hertz(nd.params.at("cutoff")), slk::dsp::Hertz(nd.params.at("sampleRate")) });
            break;
        case AppNodeType::Gain:
            h = graph.addNode(slk::filter::SimpleGainFilter<float> { slk::dsp::Db::fromLinear(nd.params.at("gain")) });
            break;
        case AppNodeType::SoftLimiter:
            h = graph.addNode(
                slk::filter::SimpleSoftLimiter<float> { slk::dsp::Db::fromLinear(nd.params.at("threshold")) });
            break;
        }

        assert(h.has_value());
        handleMap[nd.id] = *h;
    }

    for (const auto& edge : desc.edges)
        graph.connect(handleMap[edge.srcId], handleMap[edge.dstId]);

    graph.setOutput(handleMap[desc.outputNodeId]);
    graph.compile(channels, numSamples);

    return graph;
}

int main()
{
    std::cout << "=== DSP Graph Example ===\n\n";

    // ── FilterChain ──────────────────────────────────────────────────────────
    std::cout << "--- FilterChain ---\n";

    auto noiseBuf = slk::dsp::whiteNoise<float>(512, -2_dB);

    slk::filter::LowPassFilter<float>     lpf(1_kHz, 48_kHz);
    slk::filter::SimpleGainFilter<float>  gain(4_dB);
    slk::filter::SimpleSoftLimiter<float> limiter(-1_dB);

    auto chain = slk::dsp::makeChain<float>(lpf, gain, limiter);
    chain(noiseBuf);

    float maxAmp = 0.0f;

    for (const auto& s : noiseBuf)
        maxAmp = std::max(maxAmp, std::abs(s));

    std::cout << "Processed " << noiseBuf.numSamples() << " samples, max amplitude = " << maxAmp << "\n\n";

    // ── AudioGraph (direct API) ───────────────────────────────────────────────
    std::cout << "--- AudioGraph (direct) ---\n";

    // Graph topology:
    //   [input] ──► [gain x2.0] ──► [lpf 500Hz] ──► [limiter 0.9] ──► output

    slk::filter::SimpleGainFilter<float>  gainHigh(6_dB);
    slk::filter::LowPassFilter<float>     lpfLow(500_Hz, 48_kHz);
    slk::filter::SimpleSoftLimiter<float> graphLimiter(-1_dB);

    slk::dsp::AudioGraph<float> graph;

    auto hGain  = graph.addNode(gainHigh);
    auto hLow   = graph.addNode(lpfLow);
    auto hLimit = graph.addNode(graphLimiter);

    graph.connect(hGain, hLow);
    graph.connect(hLow,  hLimit);
    graph.setOutput(hLimit);
    graph.compile(1, 512);

    for (int i = 0; i < 10; ++i) {
        slk::AudioBuffer<float> buf(1, 512);

        for (auto& s : buf)
            s = 0.2f;

        graph.process(buf);

        float peak = 0.0f;

        for (const auto& s : buf)
            peak = std::max(peak, std::abs(s));

        std::cout << "Buffer " << (i + 1) << " peak = " << peak << "\n";
    }

    std::cout << "AudioGraph: processed 10 buffers\n\n";

    // ── AudioGraph (built from application descriptor) ────────────────────────
    std::cout << "--- AudioGraph (from application descriptor) ---\n";

    AppGraphDesc desc;
    desc.nodes = {
        { 1, AppNodeType::Gain,        { { "gain", 1.5f } } },
        { 2, AppNodeType::SoftLimiter, { { "threshold", 0.9f } } },
    };
    desc.edges        = { { 1, 2 } };
    desc.outputNodeId = 2;

    auto builtGraph = buildGraph(desc, 1, 512);

    slk::AudioBuffer<float> descBuf(1, 512);

    for (auto& s : descBuf)
        s = 0.5f;

    builtGraph.process(descBuf);

    float descPeak = 0.0f;

    for (const auto& s : descBuf)
        descPeak = std::max(descPeak, std::abs(s));

    std::cout << "Built from descriptor and processed, peak = " << descPeak << "\n\n";
    std::cout << "Done.\n";

    return 0;
}
