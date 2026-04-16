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

#include <tuple>

#include <slk/audiobuffer.h>
#include <slk/dsp/processor.h>

namespace slk::dsp
{

template <class T, Processor<T>... Fs>
class FilterChain
{
public:
    explicit FilterChain(Fs&... filters) : _filters { filters... }
    {
    }

    void operator()(slk::AudioBuffer<T>& buf)
    {
        std::apply([&buf](auto&... fs) { (fs(buf), ...); }, _filters);
    }

private:
    std::tuple<Fs&...> _filters;
};

template <class T, class... Fs>
FilterChain<T, Fs...> makeChain(Fs&... filters)
{
    return FilterChain<T, Fs...> { filters... };
}

} // namespace slk::dsp
