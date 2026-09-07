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
using Plan = std::unique_ptr<vc_ordered_plan, decltype(&vc_ordered_destroy)>;
struct Profile {
  vc_ordered_config c;
  uint64_t hash;
};
std::vector<Profile> Profiles() {
  std::ifstream file(ORDERED_FIXTURE);
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
class OrderedCaptures : public testing::TestWithParam<Profile> {};
TEST_P(OrderedCaptures, MatchesReviewedPublicCOutputInWholeAndReverseBands) {
  for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE}) {
    SCOPED_TRACE(target);
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
    vc_ordered_plan* raw = nullptr;
    ASSERT_EQ(vc_ordered_create_for_target(&p.c, target, &raw), VC_OK);
    Plan plan(raw, vc_ordered_destroy);
    ASSERT_EQ(vc_ordered_execute(raw, {source.data(), sp}, {whole.data(), dp}, {w, h, 0, h}), VC_OK);
    for (int y = h - 1; y >= 0; --y)
      ASSERT_EQ(vc_ordered_execute(raw, {source.data(), sp}, {bands.data(), dp}, {w, h, y, 1}), VC_OK);
    EXPECT_EQ(source, original);
    EXPECT_EQ(whole, bands);
    EXPECT_EQ(Hash(whole, p.c.depth.destination_bits, w, h, dp), p.hash);
    for (int y = 0; y < h; ++y)
      for (int x = w * db; x < dp; ++x)
        EXPECT_EQ(reinterpret_cast<uint8_t*>(whole.data())[y * dp + x], 0xcd);
  }
}
INSTANTIATE_TEST_SUITE_P(Pinned, OrderedCaptures, testing::ValuesIn(Profiles()));
TEST(OrderedContract, RejectsInvalidDescriptorsWithoutWrites) {
  vc_ordered_config c{{16, 8, 1, 1, 0}, 8};
  vc_ordered_plan* raw = nullptr;
  ASSERT_EQ(vc_ordered_create_for_target(&c, VC_TARGET_NATIVE, &raw), VC_OK);
  Plan plan(raw, vc_ordered_destroy);
  uint16_t input[2] = {0, 65535};
  uint8_t output[2] = {19, 23};
  EXPECT_EQ(vc_ordered_execute(raw, {input, 4}, {output, 1}, {2, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(output[0], 19);
  EXPECT_EQ(output[1], 23);
  EXPECT_EQ(vc_ordered_execute(raw, {nullptr, 0}, {nullptr, 0}, {2, 1, 1, 0}), VC_OK);
  EXPECT_EQ(vc_ordered_execute(raw, {input, 4}, {output, 2}, {2, 1, 1, 1}), VC_INVALID_ARGUMENT);
  for (int q : {0, 16, 17}) {
    c.quantization_bits = q;
    raw = reinterpret_cast<vc_ordered_plan*>(1);
    EXPECT_EQ(vc_ordered_create_for_target(&c, VC_TARGET_NATIVE, &raw), VC_INVALID_ARGUMENT);
    EXPECT_EQ(raw, nullptr);
  }
  c.quantization_bits = 8;
  c.depth.chroma = 2;
  EXPECT_EQ(vc_ordered_create_for_target(&c, VC_TARGET_NATIVE, &raw), VC_INVALID_ARGUMENT);
}
TEST(OrderedContract, CorrectSevenBitDifferenceThresholdAndSignedStride) {
  vc_ordered_config c{{16, 10, 1, 1, 0}, 9};
  vc_ordered_plan* raw = nullptr;
  ASSERT_EQ(vc_ordered_create_for_target(&c, VC_TARGET_NATIVE, &raw), VC_OK);
  Plan plan(raw, vc_ordered_destroy);
  // At x=0,y=9 the generated threshold is 130/2=65, never the legacy typo 75.
  std::vector<uint16_t> input(16 * 16, 60), output(16 * 16, 99);
  EXPECT_EQ(vc_ordered_execute(raw, {input.data() + 15 * 16, -32}, {output.data() + 15 * 16, -32}, {16, 16, 0, 16}),
            VC_OK);
  EXPECT_EQ(output[(15 - 9) * 16], 0);
  EXPECT_EQ(output[(15 - 1) * 16], 0);
}
TEST(OrderedContract, ExactAllocationsCoverSmallWidths) {
  for (int w : {1, 7, 16, 17, 33}) {
    vc_ordered_config c{{8, 8, 1, 1, 0}, 3};
    vc_ordered_plan* raw = nullptr;
    ASSERT_EQ(vc_ordered_create_for_target(&c, VC_TARGET_NATIVE, &raw), VC_OK);
    Plan plan(raw, vc_ordered_destroy);
    std::vector<uint8_t> input(w, 255), output(w, 0);
    ASSERT_EQ(vc_ordered_execute(raw, {input.data(), w}, {output.data(), w}, {w, 1, 0, 1}), VC_OK);
    for (auto v : output)
      EXPECT_EQ(v, 255);
  }
}
TEST(OrderedContract, RejectsUnsupportedTargetsAndMatchesExplicitNativeTarget) {
  vc_ordered_config c{{16, 8, 1, 1, 0}, 8};
  vc_ordered_plan* raw = nullptr;
  EXPECT_EQ(vc_ordered_create_for_target(&c, 3, &raw), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_ordered_create_for_target(&c, INT64_MIN, &raw), VC_INVALID_ARGUMENT);
  const auto supported = vc_ordered_supported_targets();
  if (supported) {
    ASSERT_EQ(vc_ordered_create_for_target(&c, supported & -supported, &raw), VC_OK);
    vc_ordered_destroy(raw);
  }
}
TEST(OrderedContract, AllU16CodesMatchCForIntermediateDepths) {
  const int w = 65536;
  std::vector<uint16_t> input(w), a(w), b(w);
  for (int i = 0; i < w; ++i)
    input[i] = uint16_t(i);
  for (int db = 8; db <= 16; ++db)
    for (int q = 8; q <= db && q < 16; ++q)
      for (int sf = 0; sf < 2; ++sf)
        for (int df = 0; df < 2; ++df) {
          vc_ordered_config c{{16, db, sf, df, 0}, q};
          vc_ordered_plan* raw = nullptr;
          ASSERT_EQ(vc_ordered_create(&c, &raw), VC_OK);
          Plan cp(raw, vc_ordered_destroy);
          ASSERT_EQ(vc_ordered_create_for_target(&c, VC_TARGET_NATIVE, &raw), VC_OK);
          Plan hp(raw, vc_ordered_destroy);
          const int stride = w * (db == 8 ? 1 : 2);
          std::fill(a.begin(), a.end(), 99);
          std::fill(b.begin(), b.end(), 99);
          ASSERT_EQ(vc_ordered_execute(cp.get(), {input.data(), w * 2}, {a.data(), stride}, {w, 1, 0, 1}), VC_OK);
          ASSERT_EQ(vc_ordered_execute(hp.get(), {input.data(), w * 2}, {b.data(), stride}, {w, 1, 0, 1}), VC_OK);
          EXPECT_EQ(a, b) << db << " " << q << " " << sf << " " << df;
        }
}
} // namespace
