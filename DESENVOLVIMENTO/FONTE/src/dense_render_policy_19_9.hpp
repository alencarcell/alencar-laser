#pragma once
// Alencar Laser 19.9 — non-destructive display policy, portable C++20.
// Controls only Qt preview quality and batching, never document data or CAM.
#include <cstddef>
namespace alaser::renderpolicy199 {
inline constexpr std::size_t kDenseNodeThreshold = 80000;
inline constexpr std::size_t kBatchSelectionThreshold = 32;
constexpr bool useFastDisplay(std::size_t nodes) noexcept {
 return nodes > kDenseNodeThreshold;
}
constexpr bool batchSelectionStyles(std::size_t changedObjects) noexcept {
 return changedObjects >= kBatchSelectionThreshold;
}
} // namespace alaser::renderpolicy199
