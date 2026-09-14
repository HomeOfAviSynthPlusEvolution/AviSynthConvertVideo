// Licensed under GPL version 2 or later with the inherited AviSynth linking exception.
#ifndef VIDEO_CONVERT_RESAMPLE_ALIGNED_ALLOCATOR_H
#define VIDEO_CONVERT_RESAMPLE_ALIGNED_ALLOCATOR_H
#include <cstddef>
#include <limits>
#include <new>
namespace vc::resample {
// Keep each SIMD coefficient vector inside its cache line. The throwing aligned
// allocation preserves the public plan creation API's out-of-memory contract.
template <class T>
struct AlignedAllocator {
  using value_type = T;
  AlignedAllocator() = default;
  template <class U>
  AlignedAllocator(const AlignedAllocator<U>&) noexcept {}
  T* allocate(size_t count) {
    if (count > std::numeric_limits<size_t>::max() / sizeof(T))
      throw std::bad_array_new_length();
    return static_cast<T*>(::operator new(count * sizeof(T), std::align_val_t(64)));
  }
  void deallocate(T* pointer, size_t) noexcept { ::operator delete(pointer, std::align_val_t(64)); }
};
template <class T, class U>
constexpr bool operator==(const AlignedAllocator<T>&, const AlignedAllocator<U>&) noexcept {
  return true;
}
template <class T, class U>
constexpr bool operator!=(const AlignedAllocator<T>&, const AlignedAllocator<U>&) noexcept {
  return false;
}
} // namespace vc::resample
#endif
