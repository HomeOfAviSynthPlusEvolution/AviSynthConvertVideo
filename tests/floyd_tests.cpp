// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#include "video_convert/dither.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <cstdint>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
namespace {
using Plan = std::unique_ptr<vc_floyd_context, decltype(&vc_floyd_destroy)>;
struct Profile {
  vc_floyd_config c;
  uint64_t hash;
};
std::vector<Profile> Profiles() {
  std::ifstream file(FLOYD_FIXTURE);
  std::vector<Profile> result;
  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    std::istringstream in(line);
    Profile p{};
    auto& c = p.c.depth;
    in >> c.source_bits >> c.destination_bits >> p.c.quantization_bits >> c.source_full >> c.destination_full >>
        c.chroma >> std::hex >> p.hash;
    if (!in.fail())
      result.push_back(p);
  }
  return result;
}
uint64_t Hash(const std::vector<uint16_t>& data, int bits, int w, int h, int stride) {
  uint64_t hash = 14695981039346656037ULL;
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x) {
      const int v =
          bits == 8 ? reinterpret_cast<const uint8_t*>(data.data())[y * stride + x] : data[y * stride / 2 + x];
      hash = (hash ^ uint8_t(v)) * 1099511628211ULL;
      if (bits != 8)
        hash = (hash ^ uint8_t(v >> 8)) * 1099511628211ULL;
    }
  return hash;
}
class FloydCaptures : public testing::TestWithParam<Profile> {};
TEST_P(FloydCaptures, MatchesPublicCOutputInWholeAndSequentialBands) {
  const auto p = GetParam();
  const int w = 257, h = 17, sb = p.c.depth.source_bits == 8 ? 1 : 2, db = p.c.depth.destination_bits == 8 ? 1 : 2;
  const int sp = (w * sb + 31) & ~15, dp = (w * db + 31) & ~15;
  std::vector<uint16_t> source(sp * h / 2, 0xabab), whole(dp * h / 2, 0xcdcd), bands = whole;
  const int maximum = (1 << p.c.depth.source_bits) - 1;
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x) {
      const int v = x == 0 ? 0 : x == 1 ? maximum : (x * 2573 + y * 7919 + 193) & maximum;
      if (sb == 1)
        reinterpret_cast<uint8_t*>(source.data())[y * sp + x] = uint8_t(v);
      else
        source[y * sp / 2 + x] = uint16_t(v);
    }
  const auto original = source;
  vc_floyd_context* raw = nullptr;
  ASSERT_EQ(vc_floyd_create(&p.c, w, h, &raw), VC_OK);
  Plan plan(raw, vc_floyd_destroy);
  ASSERT_EQ(vc_floyd_execute(raw, {source.data(), sp}, {whole.data(), dp}, {w, h, 0, h}), VC_OK);
  vc_floyd_reset(raw);
  for (int y = 0; y < h; ++y)
    ASSERT_EQ(vc_floyd_execute(raw, {source.data(), sp}, {bands.data(), dp}, {w, h, y, 1}), VC_OK);
  EXPECT_EQ(source, original);
  EXPECT_EQ(whole, bands);
  EXPECT_EQ(Hash(whole, p.c.depth.destination_bits, w, h, dp), p.hash);
  for (int y = 0; y < h; ++y)
    for (int x = w * db; x < dp; ++x)
      EXPECT_EQ(reinterpret_cast<uint8_t*>(whole.data())[y * dp + x], 0xcd);
}
INSTANTIATE_TEST_SUITE_P(Pinned, FloydCaptures, testing::ValuesIn(Profiles()));
TEST(FloydContract, RejectsWrongOrderBeforeStateOrOutputMutation) {
  vc_floyd_config c{{16, 8, 1, 1, 0}, 8};
  vc_floyd_context* raw = nullptr;
  ASSERT_EQ(vc_floyd_create(&c, 2, 2, &raw), VC_OK);
  Plan context(raw, vc_floyd_destroy);
  uint16_t input[] = {0, 65535, 0, 65535};
  uint8_t output[] = {19, 23, 29, 31};
  EXPECT_EQ(vc_floyd_execute(raw, {input, 4}, {output, 2}, {2, 2, 1, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_floyd_execute(raw, {input, 4}, {output, 1}, {2, 2, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(output[0], 19);
  EXPECT_EQ(output[2], 29);
  EXPECT_EQ(vc_floyd_execute(raw, {nullptr, 0}, {nullptr, 0}, {2, 2, 0, 0}), VC_OK);
  EXPECT_EQ(vc_floyd_execute(raw, {input, 4}, {output, 2}, {2, 2, 0, 1}), VC_OK);
  EXPECT_EQ(vc_floyd_execute(raw, {input, 4}, {output, 2}, {2, 2, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_floyd_execute(raw, {input, 4}, {output, 2}, {2, 2, 1, 1}), VC_OK);
  EXPECT_EQ(output[0], 0);
  EXPECT_EQ(output[1], 255);
  EXPECT_EQ(output[2], 0);
  EXPECT_EQ(output[3], 255);
  vc_floyd_reset(raw);
  EXPECT_EQ(vc_floyd_execute(raw, {input, 4}, {output, 2}, {2, 2, 0, 2}), VC_OK);
}
TEST(FloydContract, ExactAllocationsSignedStridesAndReset) {
  for (int w : {1, 7, 16, 17, 33})
    for (int q : {1, 5, 8, 15}) {
      const int h = 7;
      vc_floyd_config c{{16, 16, 1, 1, 0}, q};
      vc_floyd_context* raw = nullptr;
      ASSERT_EQ(vc_floyd_create(&c, w, h, &raw), VC_OK);
      Plan context(raw, vc_floyd_destroy);
      std::vector<uint16_t> input(w * h), reversed(w * h), out(w * h), back(w * h);
      for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
          const uint16_t v = uint16_t((x * 7919 + y * 17) & 65535);
          input[y * w + x] = v;
          reversed[(h - 1 - y) * w + x] = v;
        }
      ASSERT_EQ(vc_floyd_execute(raw, {input.data(), w * 2}, {out.data(), w * 2}, {w, h, 0, h}), VC_OK);
      vc_floyd_reset(raw);
      for (int y = 0; y < h; ++y)
        ASSERT_EQ(vc_floyd_execute(raw, {reversed.data() + (h - 1) * w, -w * 2}, {back.data() + (h - 1) * w, -w * 2},
                                   {w, h, y, 1}),
                  VC_OK);
      for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
          EXPECT_EQ(out[y * w + x], back[(h - 1 - y) * w + x]);
    }
}
TEST(FloydContract, InvalidCreationClearsOutput) {
  vc_floyd_config c{{16, 8, 1, 1, 0}, 8};
  vc_floyd_context* raw = reinterpret_cast<vc_floyd_context*>(1);
  EXPECT_EQ(vc_floyd_create(&c, 0, 1, &raw), VC_INVALID_ARGUMENT);
  EXPECT_EQ(raw, nullptr);
  c.quantization_bits = 16;
  EXPECT_EQ(vc_floyd_create(&c, 1, 1, &raw), VC_INVALID_ARGUMENT);
  c.quantization_bits = 8;
  c.depth.chroma = 2;
  EXPECT_EQ(vc_floyd_create(&c, 1, 1, &raw), VC_INVALID_ARGUMENT);
  vc_floyd_destroy(nullptr);
  vc_floyd_reset(nullptr);
}
} // namespace
