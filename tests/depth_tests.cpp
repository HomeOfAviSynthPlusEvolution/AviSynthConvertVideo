#include "video_convert/depth.h"
#include <gtest/gtest.h>
#include <algorithm>
#include <array>
#include <cstring>
#include <cmath>
#include <fstream>
#include <future>
#include <limits>
#include <memory>
#include <sstream>
#include <type_traits>
#include <vector>
namespace {
using Plan = std::unique_ptr<vc_depth_plan, decltype(&vc_depth_destroy)>;
struct Case {
  vc_depth_config config;
  uint64_t hash;
};
std::vector<Case> Cases() {
  std::ifstream file(DEPTH_FIXTURE);
  std::string line;
  std::vector<Case> cases;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    std::istringstream in(line);
    Case c{};
    if (in >> c.config.source_bits >> c.config.destination_bits >> c.config.source_full >> c.config.destination_full >>
        c.config.chroma >> std::hex >> c.hash)
      cases.push_back(c);
  }
  return cases;
}
template <class S>
S Sample(int x, int depth, bool chroma) {
  if constexpr (std::is_same_v<S, float>)
    return float((x * 73) % 769 - 128) / 512.f - (chroma ? .5f : 0.f);
  else {
    const int maximum = (1 << depth) - 1;
    return static_cast<S>(x == 0 ? 0 : x == 1 ? maximum : (x * 2573 + 193) & maximum);
  }
}
template <class D>
uint64_t Hash(const D* values, int count) {
  uint64_t hash = 14695981039346656037ULL;
  for (int x = 0; x < count; ++x) {
    uint32_t code;
    if constexpr (std::is_same_v<D, float>)
      std::memcpy(&code, values + x, 4);
    else
      code = values[x];
    for (size_t b = 0; b < sizeof(D); ++b)
      hash = (hash ^ ((code >> (8 * b)) & 255)) * 1099511628211ULL;
  }
  return hash;
}
template <class S, class D>
void Check(const Case& c, int64_t target) {
  vc_depth_plan* raw = nullptr;
  ASSERT_EQ(vc_depth_create_for_target(&c.config, target, &raw), VC_OK);
  Plan plan(raw, vc_depth_destroy);
  constexpr int width = 257, pitch = 263, height = 3;
  std::vector<S> input(pitch * height, S(7));
  std::vector<D> output(pitch * height, D(11));
  for (int y = 0; y < height; ++y)
    for (int x = 0; x < width; ++x)
      input[y * pitch + x] = Sample<S>(x, c.config.source_bits, c.config.chroma != 0);
  const auto before = input;
  const vc_const_plane src{input.data() + 2 * pitch, -ptrdiff_t(pitch * sizeof(S))};
  const vc_plane dst{output.data(), ptrdiff_t(pitch * sizeof(D))};
  for (int y : {2, 0, 1})
    ASSERT_EQ(vc_depth_execute(plan.get(), src, dst, {width, height, y, 1}), VC_OK);
  EXPECT_EQ(input, before);
  for (int y = 0; y < height; ++y) {
    EXPECT_EQ(Hash(output.data() + y * pitch, width), c.hash) << "row=" << y;
    for (int x = width; x < pitch; ++x)
      EXPECT_EQ(output[y * pitch + x], D(11));
  }
  // Exact allocations exercise the last element under ASan, not just padding.
  for (int count : {1, 7, 16, 17, 33}) {
    std::vector<S> s(input.begin(), input.begin() + count);
    std::vector<D> d(count);
    ASSERT_EQ(vc_depth_execute(plan.get(), {s.data(), ptrdiff_t(count * sizeof(S))},
                               {d.data(), ptrdiff_t(count * sizeof(D))}, {count, 1, 0, 1}),
              VC_OK);
    EXPECT_TRUE(std::equal(d.begin(), d.end(), output.begin()));
  }
}
template <class S>
void Destination(const Case& c, int64_t target) {
  if (c.config.destination_bits == 32)
    Check<S, float>(c, target);
  else if (c.config.destination_bits == 8)
    Check<S, uint8_t>(c, target);
  else
    Check<S, uint16_t>(c, target);
}
class DepthBaseline : public testing::TestWithParam<Case> {};
TEST_P(DepthBaseline, MatchesPinnedPublicScalarOutputWithRowBandsAndGuards) {
  const auto& c = GetParam();
  for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE}) {
    SCOPED_TRACE(target);
    if (c.config.source_bits == 32)
      Destination<float>(c, target);
    else if (c.config.source_bits == 8)
      Destination<uint8_t>(c, target);
    else
      Destination<uint16_t>(c, target);
  }
}
INSTANTIATE_TEST_SUITE_P(Profiles, DepthBaseline, testing::ValuesIn(Cases()),
                         [](const testing::TestParamInfo<Case>& p) {
                           const auto& c = p.param.config;
                           return "S" + std::to_string(c.source_bits) + "D" + std::to_string(c.destination_bits) +
                                  "SF" + std::to_string(c.source_full) + "DF" + std::to_string(c.destination_full) +
                                  "C" + std::to_string(c.chroma);
                         });
TEST(DepthValidation, FixtureCompleteAndInvalidRequestsDoNotWrite) {
  ASSERT_EQ(Cases().size(), 288u);
  vc_depth_config config{8, 16, 1, 1, 0};
  vc_depth_plan* raw = nullptr;
  ASSERT_EQ(vc_depth_create(&config, &raw), VC_OK);
  Plan plan(raw, vc_depth_destroy);
  uint8_t s[4] = {0, 1, 2, 3};
  uint16_t d[4] = {9, 9, 9, 9};
  EXPECT_EQ(vc_depth_execute(plan.get(), {s, 4}, {d, 8}, {4, 1, 1, 0}), VC_OK);
  EXPECT_EQ(vc_depth_execute(plan.get(), {nullptr, 0}, {nullptr, 0}, {4, 1, 0, 0}), VC_OK);
  EXPECT_EQ(vc_depth_execute(plan.get(), {s, 3}, {d, 8}, {4, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_depth_execute(plan.get(), {s, 4}, {d, 7}, {4, 1, 0, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_depth_execute(plan.get(), {s, 4}, {d, 8}, {4, 1, 1, 1}), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_depth_execute(nullptr, {s, 4}, {d, 8}, {4, 1, 0, 1}), VC_INVALID_ARGUMENT);
  for (auto value : d)
    EXPECT_EQ(value, 9);
  for (int field = 0; field < 5; ++field) {
    auto bad = config;
    switch (field) {
      case 0:
        bad.source_bits = 7;
        break;
      case 1:
        bad.destination_bits = 17;
        break;
      case 2:
        bad.source_full = -1;
        break;
      case 3:
        bad.destination_full = 2;
        break;
      default:
        bad.chroma = 2;
    }
    raw = plan.get();
    EXPECT_EQ(vc_depth_create(&bad, &raw), VC_INVALID_ARGUMENT);
    EXPECT_EQ(raw, nullptr);
  }
  EXPECT_EQ(vc_depth_create(nullptr, &raw), VC_INVALID_ARGUMENT);
  EXPECT_EQ(raw, nullptr);
  vc_depth_destroy(nullptr);
}
TEST(DepthNumerics, NonfiniteAndHugeFloatSaturateBeforeIntegerConversion) {
  const std::array<float, 8> pattern = {0, 1, 1e10f, -1e10f, INFINITY, -INFINITY, NAN, .5f};
  std::array<float, 33> s{};
  for (size_t x = 0; x < s.size(); ++x)
    s[x] = pattern[x % pattern.size()];
  for (int64_t target : {VC_TARGET_C, VC_TARGET_NATIVE})
    for (int depth : {8, 9, 10, 12, 14, 16}) {
      vc_depth_config c{32, depth, 1, 1, 0};
      vc_depth_plan* raw = nullptr;
      ASSERT_EQ(vc_depth_create_for_target(&c, target, &raw), VC_OK);
      Plan plan(raw, vc_depth_destroy);
      std::array<uint16_t, 33> d{};
      std::array<uint8_t, 33> b{};
      ASSERT_EQ(vc_depth_execute(plan.get(), {s.data(), 132},
                                 {depth == 8 ? static_cast<void*>(b.data()) : d.data(), depth == 8 ? 33 : 66},
                                 {33, 1, 0, 1}),
                VC_OK);
      const int max = (1 << depth) - 1;
      const int expected[] = {0, max, max, 0, max, 0, 0, 1 << (depth - 1)};
      for (int x = 0; x < 33; ++x)
        EXPECT_EQ(depth == 8 ? b[x] : d[x], expected[x % 8]);
    }
}
TEST(DepthNumerics, IdentityPreservesFloatBitsAndUnclippedRangeIsConcurrent) {
  vc_depth_config c{32, 32, 1, 1, 0};
  vc_depth_plan* raw = nullptr;
  ASSERT_EQ(vc_depth_create(&c, &raw), VC_OK);
  Plan identity(raw, vc_depth_destroy);
  const std::array<float, 4> s = {-0.f, NAN, INFINITY, -2.f};
  std::array<float, 4> d{};
  ASSERT_EQ(vc_depth_execute(identity.get(), {s.data(), 16}, {d.data(), 16}, {4, 1, 0, 1}), VC_OK);
  EXPECT_EQ(std::memcmp(s.data(), d.data(), 16), 0);
  c.destination_full = 0;
  ASSERT_EQ(vc_depth_create(&c, &raw), VC_OK);
  Plan plan(raw, vc_depth_destroy);
  auto run = [&] {
    std::array<float, 4> out{};
    EXPECT_EQ(vc_depth_execute(plan.get(), {s.data(), 16}, {out.data(), 16}, {4, 1, 0, 1}), VC_OK);
    return out;
  };
  auto future = std::async(std::launch::async, run);
  const auto a = run(), b = future.get();
  EXPECT_LT(a[3], 0);
  EXPECT_EQ(a[3], b[3]);
  EXPECT_TRUE(std::isnan(a[1]));
  EXPECT_TRUE(std::isinf(a[2]));
}
} // namespace

TEST(DepthTargets, SelectionIsLocalAndRejectsInvalidBits) {
  const vc_depth_config c{8, 16, 1, 1, 0};
  vc_depth_plan* raw = nullptr;
  EXPECT_EQ(vc_depth_create_for_target(&c, -2, &raw), VC_INVALID_ARGUMENT);
  EXPECT_EQ(vc_depth_create_for_target(&c, 3, &raw), VC_INVALID_ARGUMENT);
  auto mask = vc_depth_supported_targets();
  EXPECT_GE(mask, 0);
  if (mask) {
    ASSERT_EQ(vc_depth_create_for_target(&c, mask & -mask, &raw), VC_OK);
    vc_depth_destroy(raw);
  }
}
TEST(DepthNumerics, ExhaustiveU16CodesMatchCForAllRangeAndChromaCombinations) {
  std::vector<uint16_t> source(65536), scalar(65536), native(65536);
  for (size_t x = 0; x < source.size(); ++x)
    source[x] = uint16_t(x);
  for (int bits : {9, 10, 11, 12, 13, 14, 15, 16})
    for (int sf : {0, 1})
      for (int df : {0, 1})
        for (int chroma : {0, 1}) {
          SCOPED_TRACE(::testing::Message() << bits << sf << df << chroma);
          const vc_depth_config c{16, bits, sf, df, chroma};
          vc_depth_plan *a = nullptr, *b = nullptr;
          ASSERT_EQ(vc_depth_create(&c, &a), VC_OK);
          Plan cp(a, vc_depth_destroy);
          ASSERT_EQ(vc_depth_create_for_target(&c, VC_TARGET_NATIVE, &b), VC_OK);
          Plan np(b, vc_depth_destroy);
          ASSERT_EQ(vc_depth_execute(cp.get(), {source.data(), 131072}, {scalar.data(), 131072}, {65536, 1, 0, 1}),
                    VC_OK);
          ASSERT_EQ(vc_depth_execute(np.get(), {source.data(), 131072}, {native.data(), 131072}, {65536, 1, 0, 1}),
                    VC_OK);
          EXPECT_EQ(scalar, native);
        }
}
