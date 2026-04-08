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
#include <vector>
#include <atomic>
#include <span>
#include <bit>

namespace slk
{

//! Needs to be initialized with powers of 2 otherwise can have "hidden writes" due to differents in present and
//! expeceted capacity
template <class T>
class alignas(64) RingBuffer // NOLINT(clang-analyzer-optin.performance.Padding)
{
public:
    RingBuffer(const size_t bufferSize)
    {
        auto size = bufferSize;

        if (!std::has_single_bit(bufferSize)) {
            size = std::bit_ceil(bufferSize);
        }

        _data.resize(size);
        _writeIx.store(0, std::memory_order_relaxed);
        _readIx.store(0, std::memory_order_relaxed);
    }

    size_t write(std::span<const T> data)
    {
        const auto sizeToWrite = std::min<size_t>(canWrite(), data.size());

        if (sizeToWrite == 0) {
            return 0;
        }

        const auto writeIx = _writeIx.load(std::memory_order_relaxed);
        const auto maskedIx = writeIx & mask();
        const auto firstWrite = std::min<size_t>(sizeToWrite, _data.capacity() - maskedIx);
        const auto secondWrite = sizeToWrite - firstWrite;

        std::copy_n(data.begin(), firstWrite, _data.begin() + maskedIx);

        if (secondWrite > 0) {
            std::copy_n(data.begin() + firstWrite, secondWrite, _data.begin());
        }

        _writeIx.store(writeIx + sizeToWrite, std::memory_order_release);

        return sizeToWrite;
    }

    size_t read(std::span<T> dest, const size_t size)
    {
        const auto sizeToRead = std::min<size_t>(canRead(), size);

        if (sizeToRead == 0) {
            return 0;
        }

        const auto readIx = _readIx.load(std::memory_order_relaxed);
        const auto maskedIx = readIx & mask();
        const auto firstRead = std::min<size_t>(sizeToRead, _data.capacity() - maskedIx);
        const auto secondRead = sizeToRead - firstRead;

        std::copy_n(_data.begin() + maskedIx, firstRead, dest.begin());

        if (secondRead > 0) {
            std::copy_n(_data.begin(), secondRead, dest.begin() + firstRead);
        }

        _readIx.store(readIx + sizeToRead, std::memory_order_release);

        return sizeToRead;
    }

    size_t peek(std::span<T> dest, const size_t size)
    {
        const auto sizeToPeek = std::min<size_t>(canRead(), size);

        if (sizeToPeek == 0) {
            return 0;
        }

        const auto peekIx = _readIx.load(std::memory_order_relaxed);
        const auto maskedIx = peekIx & mask();
        const auto firstPeek = std::min<size_t>(sizeToPeek, _data.capacity() - maskedIx);
        const auto secondPeek = sizeToPeek - firstPeek;

        std::copy_n(_data.begin() + maskedIx, firstPeek, dest.begin());

        if (secondPeek > 0) {
            std::copy_n(_data.begin(), secondPeek, dest.begin() + firstPeek);
        }

        return sizeToPeek;
    }

    size_t canRead() const
    {
        const auto writeIx = _writeIx.load(std::memory_order_acquire);
        const auto readIx = _readIx.load(std::memory_order_acquire);
        return writeIx - readIx;
    }

    size_t canWrite() const
    {
        return _data.capacity() - canRead() - 1;
    }

private:
    size_t mask() const
    {
        return _data.capacity() - 1;
    }

    std::vector<T> _data;
    alignas(64) std::atomic<size_t> _writeIx;
    alignas(64) std::atomic<size_t> _readIx;
};

}
