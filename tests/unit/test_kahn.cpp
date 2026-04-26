#include <gtest/gtest.h>

#include <slk/dsp/kahn.h>

#include <algorithm>
#include <vector>

using slk::dsp::KahnTopologicalSort;

namespace {

// True iff `pred` appears before `succ` in `order`.
bool precedes(const std::vector<size_t>& order, size_t pred, size_t succ)
{
    auto pi = std::find(order.begin(), order.end(), pred);
    auto si = std::find(order.begin(), order.end(), succ);
    return pi != order.end() && si != order.end() && pi < si;
}

} // namespace

TEST(KahnSort, EmptyGraph)
{
    KahnTopologicalSort sorter;
    auto order = sorter(0, {});
    ASSERT_TRUE(order.has_value());
    EXPECT_TRUE(order->empty());
}

TEST(KahnSort, SingleNode)
{
    KahnTopologicalSort sorter;
    auto order = sorter(1, { {} });
    ASSERT_TRUE(order.has_value());
    ASSERT_EQ(order->size(), 1u);
    EXPECT_EQ((*order)[0], 0u);
}

TEST(KahnSort, IndependentNodes)
{
    KahnTopologicalSort sorter;
    auto order = sorter(3, { {}, {}, {} });
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->size(), 3u);
}

TEST(KahnSort, LinearChain)
{
    // 0 -> 1 -> 2 -> 3
    // inputAdj[i] = predecessors of i
    KahnTopologicalSort sorter;
    auto order = sorter(4, { {}, { 0 }, { 1 }, { 2 } });
    ASSERT_TRUE(order.has_value());
    ASSERT_EQ(order->size(), 4u);

    EXPECT_TRUE(precedes(*order, 0, 1));
    EXPECT_TRUE(precedes(*order, 1, 2));
    EXPECT_TRUE(precedes(*order, 2, 3));
}

TEST(KahnSort, Diamond)
{
    // 0 -> 1, 0 -> 2, 1 -> 3, 2 -> 3
    KahnTopologicalSort sorter;
    auto order = sorter(4, { {}, { 0 }, { 0 }, { 1, 2 } });
    ASSERT_TRUE(order.has_value());
    ASSERT_EQ(order->size(), 4u);

    EXPECT_TRUE(precedes(*order, 0, 1));
    EXPECT_TRUE(precedes(*order, 0, 2));
    EXPECT_TRUE(precedes(*order, 1, 3));
    EXPECT_TRUE(precedes(*order, 2, 3));
}

TEST(KahnSort, MultipleSinks)
{
    // 0 -> 1, 0 -> 2, 0 -> 3 — three independent leaves
    KahnTopologicalSort sorter;
    auto order = sorter(4, { {}, { 0 }, { 0 }, { 0 } });
    ASSERT_TRUE(order.has_value());
    ASSERT_EQ(order->size(), 4u);
    EXPECT_EQ((*order)[0], 0u); // 0 must come first
}

TEST(KahnSort, CycleReturnsNullopt)
{
    // 0 -> 1, 1 -> 0
    KahnTopologicalSort sorter;
    auto order = sorter(2, { { 1 }, { 0 } });
    EXPECT_FALSE(order.has_value());
}

TEST(KahnSort, ThreeNodeCycleReturnsNullopt)
{
    // 0 -> 1 -> 2 -> 0
    KahnTopologicalSort sorter;
    auto order = sorter(3, { { 2 }, { 0 }, { 1 } });
    EXPECT_FALSE(order.has_value());
}

TEST(KahnSort, SelfLoopReturnsNullopt)
{
    KahnTopologicalSort sorter;
    auto order = sorter(1, { { 0 } });
    EXPECT_FALSE(order.has_value());
}

TEST(KahnSort, PartialCycleReturnsNullopt)
{
    // 0 -> 1, 1 -> 2, 2 -> 1 — the {1,2} sub-cycle blocks the sort even
    // though node 0 is acyclic.
    KahnTopologicalSort sorter;
    auto order = sorter(3, { {}, { 0, 2 }, { 1 } });
    EXPECT_FALSE(order.has_value());
}

TEST(KahnSort, OrderIsValidPermutation)
{
    KahnTopologicalSort sorter;
    auto order = sorter(5, { {}, { 0 }, { 0 }, { 1, 2 }, { 3 } });
    ASSERT_TRUE(order.has_value());
    ASSERT_EQ(order->size(), 5u);

    std::vector<size_t> sorted = *order;
    std::ranges::sort(sorted);
    for (size_t i = 0; i < sorted.size(); ++i) {
        EXPECT_EQ(sorted[i], i) << "order is not a permutation of [0..n)";
    }
}
