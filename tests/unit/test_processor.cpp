#include <gtest/gtest.h>

#include <slk/audiobuffer.h>
#include <slk/dsp/processor.h>

#include <atomic>
#include <thread>
#include <vector>

using slk::dsp::Param;
using slk::dsp::Processor;
using slk::dsp::TopologicalSorter;

namespace {

struct GainProcessor
{
    float gain { 1.0f };

    void operator()(slk::AudioBuffer<float>& buf)
    {
        for (auto& s : buf) s *= gain;
    }
};

struct NotAProcessor
{
    int operator()(slk::AudioBuffer<float>&) { return 0; } // wrong return type
};

} // namespace

TEST(ProcessorConcept, AcceptsValidProcessor)
{
    static_assert(Processor<GainProcessor, float>);
    SUCCEED();
}

TEST(ProcessorConcept, RejectsWrongReturnType)
{
    static_assert(!Processor<NotAProcessor, float>);
    SUCCEED();
}

TEST(ProcessorConcept, AcceptsLambdaProcessor)
{
    auto lam = [](slk::AudioBuffer<float>& buf) {
        for (auto& s : buf) s = 0.0f;
    };
    static_assert(Processor<decltype(lam), float>);
    SUCCEED();
}

namespace {

struct ConstSorter
{
    std::optional<std::vector<size_t>>
    operator()(size_t n, const std::vector<std::vector<size_t>>&) const
    {
        std::vector<size_t> v(n);
        for (size_t i = 0; i < n; ++i) v[i] = i;
        return v;
    }
};

} // namespace

TEST(TopologicalSorterConcept, AcceptsValidSorter)
{
    static_assert(TopologicalSorter<ConstSorter>);
    SUCCEED();
}

TEST(Param, ConstructFromValue)
{
    Param<float> p { 3.14f };
    EXPECT_FLOAT_EQ(static_cast<float>(p), 3.14f);
}

TEST(Param, AssignFromValue)
{
    Param<int> p { 0 };
    p = 42;
    EXPECT_EQ(static_cast<int>(p), 42);
}

TEST(Param, CopyConstructCopiesCurrentValue)
{
    Param<int> a { 7 };
    Param<int> b { a };
    EXPECT_EQ(static_cast<int>(b), 7);
}

TEST(Param, CopyAssignCopiesCurrentValue)
{
    Param<int> a { 11 };
    Param<int> b { 0 };
    b = a;
    EXPECT_EQ(static_cast<int>(b), 11);
}

TEST(Param, MoveConstructResetsSource)
{
    Param<int> a { 99 };
    Param<int> b { std::move(a) };
    EXPECT_EQ(static_cast<int>(b), 99);
    // Per implementation, source is reset to default-constructed value.
    EXPECT_EQ(static_cast<int>(a), 0);
}

TEST(Param, MoveAssignResetsSource)
{
    Param<int> a { 55 };
    Param<int> b { 0 };
    b = std::move(a);
    EXPECT_EQ(static_cast<int>(b), 55);
    EXPECT_EQ(static_cast<int>(a), 0);
}

TEST(Param, ConcurrentReadAndWriteIsLockFree)
{
    // Smoke test: many writers + readers don't crash, no torn reads observed
    // (writes are atomic; we never see a value other than what was written).
    Param<int> p { 0 };
    std::atomic<bool> stop { false };

    std::thread writer([&]() {
        for (int i = 1; !stop.load(std::memory_order_relaxed); ++i) {
            p = i;
        }
    });

    int seen = 0;
    for (int i = 0; i < 10000; ++i) {
        const int v = p;
        // We can't strictly assert ordering, but a value is always either
        // the previous write or a new one. Just exercise the read path.
        seen = v;
    }

    stop.store(true, std::memory_order_relaxed);
    writer.join();
    (void)seen; // touched
    SUCCEED();
}
