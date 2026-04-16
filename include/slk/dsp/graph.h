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

#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <optional>
#include <vector>

#include <slk/audiobuffer.h>
#include <slk/dsp/kahn.h>
#include <slk/dsp/processor.h>

namespace slk::dsp
{

template <class T>
struct GraphNode
{
    std::function<void(slk::AudioBuffer<T>&)> processor {};
    std::vector<size_t> inputIndices;

    slk::AudioBuffer<T> workBuffer {};
    slk::AudioBuffer<T>* activeBuffer { nullptr };
    size_t outputCount { 0 };
};

template <class T>
class AudioGraph
{
public:
    using NodeHandle = size_t;

    AudioGraph() = default;
    ~AudioGraph() = default;

    AudioGraph(const AudioGraph&) = delete;
    AudioGraph& operator=(const AudioGraph&) = delete;

    AudioGraph(AudioGraph&&) = default;
    AudioGraph& operator=(AudioGraph&&) = default;

    template <Processor<T> P>
    NodeHandle addNode(P proc)
    {
        _nodes.emplace_back();
        _nodes.back().processor = std::move(proc);
        return _nodes.size() - 1;
    }

    void connect(NodeHandle src, NodeHandle dst)
    {
        assert(src < _nodes.size() && dst < _nodes.size());
        _nodes[dst].inputIndices.push_back(src);
    }

    void setOutput(NodeHandle h)
    {
        assert(h < _nodes.size());
        _outputHandle = h;
    }

    template <TopologicalSorter Sorter = KahnTopologicalSort>
    bool compile(uint32_t channels, uint32_t numSamples, Sorter sorter = {})
    {
        assert(_outputHandle.has_value());

        _channels = channels;
        _numSamples = numSamples;

        const size_t nodeCount = _nodes.size();

        //topological sort — returns node indices in dependency order
        std::vector<std::vector<size_t>> inputAdj(nodeCount);

        std::ranges::transform(_nodes, inputAdj.begin(), [](const auto& n) {
            return n.inputIndices;
        });

        auto perm = sorter(nodeCount, inputAdj);

        if (!perm.has_value()) {
            return false;
        }

        // reorder _nodes into sorted order
        std::vector<GraphNode<T>> sorted;
        sorted.reserve(nodeCount);

        for (auto idx : perm.value()) {
            sorted.push_back(std::move(_nodes[idx]));
        }

        _nodes = std::move(sorted);

        // build inverse permutation and remap indices
        std::vector<size_t> inv(nodeCount);

        for (size_t i = 0; i < nodeCount; ++i) {
            inv[perm.value()[i]] = i;
        }

        _outputHandle = inv[_outputHandle.value()];

        // remap indices and compute outputCount
        for (auto& node : _nodes) {
            node.outputCount = 0;

            std::ranges::transform(node.inputIndices, node.inputIndices.begin(), [&inv](size_t idx) {
                return inv[idx];
            });

            for (auto idx : node.inputIndices) {
                ++_nodes[idx].outputCount;
            }
        }

        // assign activeBuffer
        for (auto& node : _nodes) {
            bool canShare = node.inputIndices.size() == 1
                && _nodes[node.inputIndices[0]].outputCount == 1;

            if (canShare) {
                node.activeBuffer = _nodes[node.inputIndices[0]].activeBuffer;
            } else {
                node.workBuffer.setSize(channels, numSamples);
                node.activeBuffer = &node.workBuffer;
            }
        }

        return true;
    }

    void process(slk::AudioBuffer<T>& buf)
    {
        assert(_outputHandle.has_value());

        if (!_outputHandle.has_value()) {
            return;
        }

        for (auto& node : _nodes) {
            if (node.inputIndices.empty()) {
                node.workBuffer = buf;
            } else if (node.inputIndices.size() == 1) {
                auto* predBuffer = _nodes[node.inputIndices[0]].activeBuffer;

                if (!predBuffer) {
                    return;
                }

                if (node.activeBuffer != predBuffer) {
                    node.workBuffer = *predBuffer;
                }
            } else {
                node.workBuffer.clear();

                for (auto idx : node.inputIndices) {
                    node.workBuffer += *_nodes[idx].activeBuffer;
                }
            }

            if (node.processor) {
                node.processor(*node.activeBuffer);
            }
        }

        buf = *_nodes[_outputHandle.value()].activeBuffer;
    }

    auto asCallback()
    {
        return [this](slk::AudioBuffer<T>& buf) { process(buf); };
    }

private:
    std::vector<GraphNode<T>> _nodes {};
    std::optional<NodeHandle> _outputHandle;
    uint32_t _channels { 0 };
    uint32_t _numSamples { 0 };
};

} // namespace slk::dsp
