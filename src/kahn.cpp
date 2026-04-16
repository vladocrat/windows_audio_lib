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

#include <slk/dsp/kahn.h>

#include <algorithm>
#include <queue>

namespace slk::dsp
{

std::optional<std::vector<size_t>>
KahnTopologicalSort::operator()(size_t nodeCount, const std::vector<std::vector<size_t>>& inputAdj) const
{
    std::vector<size_t> inDeg(nodeCount);

    std::ranges::transform(inputAdj, inDeg.begin(), [](const auto& inputs) { return inputs.size(); });

    std::vector<std::vector<size_t>> children(nodeCount);

    for (size_t i = 0; i < nodeCount; ++i) {
        for (auto pred : inputAdj[i]) {
            children[pred].push_back(i);
        }
    }

    std::queue<size_t> q;

    for (size_t i = 0; i < nodeCount; ++i) {
        if (inDeg[i] == 0) {
            q.push(i);
        }
    }

    std::vector<size_t> order;
    order.reserve(nodeCount);

    while (!q.empty()) {
        const size_t cur = q.front();
        q.pop();
        order.push_back(cur);

        for (const size_t child : children[cur]) {
            if (--inDeg[child] == 0) {
                q.push(child);
            }
        }
    }

    if (order.size() != nodeCount) {
        return std::nullopt;
    }

    return order;
}

} // namespace slk::dsp
