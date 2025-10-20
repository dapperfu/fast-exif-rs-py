#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <string>

namespace hefraw {

// Export 16-bit CFA to DNG (lossless) for validation in open tools
bool export_cfa16_to_dng(const std::vector<uint16_t>& cfa, uint32_t width, uint32_t height,
                         uint8_t cfa_pattern, const std::string& output_path);

// Export 16-bit CFA to 16-bit TIFF for quick viewing/testing
bool export_cfa16_to_tiff(const std::vector<uint16_t>& cfa, uint32_t width, uint32_t height,
                          const std::string& output_path);

} // namespace hefraw
