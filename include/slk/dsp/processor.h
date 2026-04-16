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

#include <atomic>
#include <optional>
#include <vector>

#include <slk/audiobuffer.h>

namespace slk::dsp
{

template <class P, class T>
concept Processor = requires(P& p, slk::AudioBuffer<T>& buf) {
    { p(buf) } -> std::same_as<void>;
};

template <class S>
concept TopologicalSorter = requires(const S& s, size_t nodeCount, const std::vector<std::vector<size_t>>& inputAdj) {
    { s(nodeCount, inputAdj) } -> std::same_as<std::optional<std::vector<size_t>>>;
};

template <class T>
struct Param final
{
    Param(T v) : _value { v }
    {
    }

    Param(const Param& other) : _value { other._value.load(std::memory_order_relaxed) }
    {
    }

    Param(Param&& other) noexcept : _value { other._value.load(std::memory_order_relaxed) }
    {
        other._value.store(T {}, std::memory_order_relaxed);
    }

    Param& operator=(const Param& other)
    {
        if (this != &other) {
            _value.store(other._value.load(std::memory_order_relaxed), std::memory_order_relaxed);
        }

        return *this;
    }

    Param& operator=(Param&& other) noexcept
    {
        if (this != &other) {
            _value.store(other._value.load(std::memory_order_relaxed), std::memory_order_relaxed);
            other._value.store(T {}, std::memory_order_relaxed);
        }

        return *this;
    }

    Param& operator=(T v)
    {
        _value.store(v, std::memory_order_relaxed);
        return *this;
    }

    operator T() const noexcept
    {
        return _value.load(std::memory_order_relaxed);
    }

    ~Param() noexcept = default;

private:
    std::atomic<T> _value;
};

} // namespace slk::dsp
