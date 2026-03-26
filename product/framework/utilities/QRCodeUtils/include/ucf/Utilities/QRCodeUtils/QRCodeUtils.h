#pragma once

#include "ucf/Utilities/UtilitiesCommonFile/UtilitiesExport.h"
#include <string>
#include <vector>
#include <cstdint>

namespace ucf {
namespace Utilities {

/**
 * @brief QR Code generation utility class
 * 
 * This class provides a thin wrapper around the underlying QR code generation library,
 * allowing the implementation to be swapped without affecting client code.
 */
class Utilities_EXPORT QRCodeUtils {
public:
    /**
     * @brief Error correction level for QR code
     */
    enum class ErrorCorrectionLevel {
        Low,      // ~7% error correction
        Medium,   // ~15% error correction
        Quartile, // ~25% error correction
        High      // ~30% error correction
    };

    /**
     * @brief Generate QR code from text string
     * 
     * @param text Input text to encode
     * @param ecl Error correction level (default: Medium)
     * @return 2D boolean matrix representing the QR code (true = dark, false = light)
     *         Returns empty vector if generation fails
     */
    static std::vector<std::vector<bool>> generateFromText(const std::string& text, ErrorCorrectionLevel ecl = ErrorCorrectionLevel::Medium);

    /**
     * @brief Generate QR code from binary data
     * 
     * @param data Binary data to encode
     * @param ecl Error correction level (default: Medium)
     * @return 2D boolean matrix representing the QR code (true = dark, false = light)
     *         Returns empty vector if generation fails
     */
    static std::vector<std::vector<bool>> generateFromBinary(const std::vector<uint8_t>& data, ErrorCorrectionLevel ecl = ErrorCorrectionLevel::Medium);

    /**
     * @brief Convert QR code matrix to SVG string
     * 
     * @param qrCode 2D boolean matrix (from generateFromText or generateFromBinary)
     * @param border Border size in modules (default: 4)
     * @param darkColor Color for dark modules in CSS format (default: "#000000")
     * @param lightColor Color for light modules/background in CSS format (default: "#FFFFFF")
     * @return SVG string representation, or empty string if input is invalid
     * 
     * @note Color formats supported: hex (#RGB, #RRGGBB), rgb(r,g,b), rgba(r,g,b,a), named colors, etc.
     */
    static std::string toSvgString(const std::vector<std::vector<bool>>& qrCode, int border = 4, const std::string& darkColor = "#000000", const std::string& lightColor = "#FFFFFF");

    /**
     * @brief Get the size (width/height) of the QR code matrix
     * 
     * @param qrCode 2D boolean matrix
     * @return Size in modules, or 0 if invalid
     */
    static int getSize(const std::vector<std::vector<bool>>& qrCode);

    /**
     * @brief Check if a specific module is dark
     * 
     * @param qrCode 2D boolean matrix
     * @param x X coordinate (0-based)
     * @param y Y coordinate (0-based)
     * @return true if dark module, false if light or out of bounds
     */
    static bool isDarkModule(const std::vector<std::vector<bool>>& qrCode, int x, int y);
};

} // namespace Utilities
} // namespace ucf