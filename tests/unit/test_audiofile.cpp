#include <gtest/gtest.h>

#include <slk/audiobuffer.h>
#include <slk/audiofile.h>
#include <slk/audioformat.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using slk::Access;
using slk::AudioFormat;
using slk::File;
using slk::WAV;

namespace {

class TempPath
{
public:
    TempPath()
    {
        const auto tmp = std::filesystem::temp_directory_path()
                       / ("audio_lib_test_" + std::to_string(uniqueId()) + ".wav");
        _path = tmp.string();
    }

    ~TempPath()
    {
        std::error_code ec;
        std::filesystem::remove(_path, ec);
    }

    const std::string& path() const { return _path; }

private:
    static unsigned uniqueId()
    {
        static std::atomic<unsigned> counter { 0 };
        return ++counter;
    }

    std::string _path;
};

// Minimal WAV writer that emits a 16-bit PCM mono file. We can't use
// slk::WAV::write here because it's a stub; this helper builds the bytes
// directly so the read path under test sees a real, valid file.
void writeMinimalPcm16Wav(const std::string& path,
                          uint32_t sampleRate,
                          uint16_t numChannels,
                          const std::vector<int16_t>& samples)
{
    std::ofstream f(path, std::ios::binary);
    ASSERT_TRUE(f.is_open()) << "cannot open " << path;

    const uint32_t dataSize    = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
    const uint32_t blockAlign  = static_cast<uint16_t>(numChannels * 2);
    const uint32_t byteRate    = sampleRate * blockAlign;
    const uint32_t fileSize    = 4 + (8 + 16) + (8 + dataSize); // "WAVE" + fmt + data

    auto write = [&](const void* p, size_t n) {
        f.write(reinterpret_cast<const char*>(p), static_cast<std::streamsize>(n));
    };

    write("RIFF", 4);
    write(&fileSize, 4);
    write("WAVE", 4);

    write("fmt ", 4);
    const uint32_t fmtSize  = 16;
    const uint16_t pcmFmt   = 1;
    const uint16_t bps      = 16;
    write(&fmtSize, 4);
    write(&pcmFmt, 2);
    write(&numChannels, 2);
    write(&sampleRate, 4);
    write(&byteRate, 4);
    write(&blockAlign, 2);
    write(&bps, 2);

    write("data", 4);
    write(&dataSize, 4);
    write(samples.data(), dataSize);
}

} // namespace

// ── File ─────────────────────────────────────────────────────────────────────

TEST(File, OpenForWriteCreatesFile)
{
    TempPath tmp;
    {
        File f(tmp.path(), Access::Write);
        EXPECT_TRUE(f.isOpen());
    }
    EXPECT_TRUE(std::filesystem::exists(tmp.path()));
}

TEST(File, OpenForReadOnMissingFileReturnsClosed)
{
    File f("__definitely_missing_path_/abc.bin", Access::Read);
    EXPECT_FALSE(f.isOpen());
}

TEST(File, CloseFlipsIsOpen)
{
    TempPath tmp;
    File f(tmp.path(), Access::Write);
    ASSERT_TRUE(f.isOpen());
    EXPECT_TRUE(f.close());
    EXPECT_FALSE(f.isOpen());
}

TEST(File, ReadWritePreservesBytes)
{
    TempPath tmp;
    {
        std::ofstream out(tmp.path(), std::ios::binary);
        const char payload[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
        out.write(payload, sizeof(payload));
    }

    File f(tmp.path(), Access::Read);
    ASSERT_TRUE(f.isOpen());

    char buf[5] {};
    f.read(buf, 5);
    EXPECT_EQ(buf[0], 0x01);
    EXPECT_EQ(buf[4], 0x05);
}

TEST(File, OpenForReadWriteOpensExistingFile)
{
    TempPath tmp;
    {
        std::ofstream out(tmp.path(), std::ios::binary);
        out.put('A');
    }

    File f(tmp.path(), Access::ReadWrite);
    EXPECT_TRUE(f.isOpen());
}

TEST(File, ReadIntoSpanReadsBytes)
{
    TempPath tmp;
    {
        std::ofstream out(tmp.path(), std::ios::binary);
        const char payload[] = { 'h', 'e', 'l', 'l', 'o' };
        out.write(payload, sizeof(payload));
    }

    File f(tmp.path(), Access::Read);
    ASSERT_TRUE(f.isOpen());

    char raw[5] {};
    std::span<char> dst { raw, 5 };
    f.read(dst, 5);

    EXPECT_EQ(raw[0], 'h');
    EXPECT_EQ(raw[4], 'o');
}

TEST(File, SkipAdvancesReadPosition)
{
    TempPath tmp;
    {
        std::ofstream out(tmp.path(), std::ios::binary);
        const char payload[] = { 0x10, 0x20, 0x30, 0x40 };
        out.write(payload, sizeof(payload));
    }

    File f(tmp.path(), Access::Read);
    ASSERT_TRUE(f.isOpen());

    f.skip(2);
    char buf[2] {};
    f.read(buf, 2);
    EXPECT_EQ(buf[0], 0x30);
    EXPECT_EQ(buf[1], 0x40);
}

// ── WAV ──────────────────────────────────────────────────────────────────────

TEST(WAV, OpenMissingFileReportsClosed)
{
    WAV wav("__definitely_missing_path_/missing.wav", Access::Read);
    EXPECT_FALSE(wav.isOpen());
}

TEST(WAV, ReadHeaderFieldsRoundtrip)
{
    TempPath tmp;
    writeMinimalPcm16Wav(tmp.path(), 44100, 1, { 0, 100, 200, -100, -200 });

    WAV wav(tmp.path(), Access::Read);
    ASSERT_TRUE(wav.isOpen());
    ASSERT_TRUE(wav.read());

    const auto& h = wav.header();
    EXPECT_EQ(h.numChannels, 1u);
    EXPECT_EQ(h.sampleRateHz, 44100u);
    EXPECT_EQ(h.bitsPerSample, 16u);
    EXPECT_EQ(h.audioFormat, 1u);    // PCM
    EXPECT_EQ(h.dataSize, 5u * sizeof(int16_t));
}

TEST(WAV, FormatReflectsHeader)
{
    TempPath tmp;
    writeMinimalPcm16Wav(tmp.path(), 48000, 2, { 1, 2, 3, 4 });

    WAV wav(tmp.path(), Access::Read);
    ASSERT_TRUE(wav.isOpen());
    ASSERT_TRUE(wav.read());

    auto fmt = wav.format();
    EXPECT_EQ(fmt.channels(), 2u);
    EXPECT_EQ(fmt.sampleRate(), 48000u);
    EXPECT_EQ(fmt.bitsPerSample(), 16u);
    EXPECT_EQ(fmt.type(), AudioFormat::Type::PCM);
}

TEST(WAV, PayloadContainsRawSampleBytes)
{
    TempPath tmp;
    const std::vector<int16_t> samples { 0x1234, 0x5678, 0x7FFF, -0x8000 };
    writeMinimalPcm16Wav(tmp.path(), 22050, 1, samples);

    WAV wav(tmp.path(), Access::Read);
    ASSERT_TRUE(wav.isOpen());
    ASSERT_TRUE(wav.read());

    const auto& payload = wav.payload();
    ASSERT_EQ(payload.size(), samples.size() * sizeof(int16_t));

    // payload is AudioBuffer<char> — bytes laid out in the same order as
    // the file. Verify the first sample's two bytes match little-endian.
    const auto& bytes = payload.data();
    EXPECT_EQ(static_cast<unsigned char>(bytes[0]), 0x34u);
    EXPECT_EQ(static_cast<unsigned char>(bytes[1]), 0x12u);
}

TEST(WAV, ReadSkipsUnknownChunks)
{
    // Build a WAV with an unknown "JUNK" chunk between "fmt " and "data".
    // The parser's else-branch should skip it and still find both required
    // chunks.
    TempPath tmp;
    {
        std::ofstream f(tmp.path(), std::ios::binary);
        ASSERT_TRUE(f.is_open());

        auto write = [&](const void* p, size_t n) {
            f.write(reinterpret_cast<const char*>(p), static_cast<std::streamsize>(n));
        };

        const uint32_t junkPayloadSize = 6;
        const uint32_t dataSize        = 4; // 2 mono int16 samples
        const uint32_t fileSize =
            4 + (8 + 16) + (8 + junkPayloadSize) + (8 + dataSize);

        write("RIFF", 4); write(&fileSize, 4); write("WAVE", 4);

        // fmt
        write("fmt ", 4);
        uint32_t fmtSize = 16; write(&fmtSize, 4);
        uint16_t pcmFmt = 1, ch = 1, bps = 16; uint32_t sr = 8000, br = 16000;
        uint16_t blk = 2;
        write(&pcmFmt, 2); write(&ch, 2); write(&sr, 4); write(&br, 4);
        write(&blk, 2); write(&bps, 2);

        // junk chunk (unknown)
        write("JUNK", 4); write(&junkPayloadSize, 4);
        const char filler[junkPayloadSize] = { 'a', 'b', 'c', 'd', 'e', 'f' };
        write(filler, junkPayloadSize);

        // data
        write("data", 4); write(&dataSize, 4);
        const int16_t samples[2] = { 100, -100 };
        write(samples, dataSize);
    }

    WAV wav(tmp.path(), Access::Read);
    ASSERT_TRUE(wav.isOpen());
    ASSERT_TRUE(wav.read());

    const auto& h = wav.header();
    EXPECT_EQ(h.numChannels, 1u);
    EXPECT_EQ(h.sampleRateHz, 8000u);
    EXPECT_EQ(h.dataSize, 4u);
}
