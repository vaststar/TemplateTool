#include "ucf/Utilities/QRCodeUtils/QRCodeUtils.h"
#include "qrcodegen.hpp"
#include <sstream>
#include <stdexcept>

namespace ucf::Utilities
{

namespace
{
    // Helper function to convert our error correction level to qrcodegen's
    qrcodegen::QrCode::Ecc toQrCodegenEcc(QRCodeUtils::ErrorCorrectionLevel ecl)
    {
        switch (ecl) {
            case QRCodeUtils::ErrorCorrectionLevel::Low:
                return qrcodegen::QrCode::Ecc::LOW;
            case QRCodeUtils::ErrorCorrectionLevel::Medium:
                return qrcodegen::QrCode::Ecc::MEDIUM;
            case QRCodeUtils::ErrorCorrectionLevel::Quartile:
                return qrcodegen::QrCode::Ecc::QUARTILE;
            case QRCodeUtils::ErrorCorrectionLevel::High:
                return qrcodegen::QrCode::Ecc::HIGH;
            default:
                return qrcodegen::QrCode::Ecc::MEDIUM;
        }
    }

    // Helper function to convert qrcodegen QR code to our matrix format
    std::vector<std::vector<bool>> convertToMatrix(const qrcodegen::QrCode& qr)
    {
        int size = qr.getSize();
        std::vector<std::vector<bool>> result(size, std::vector<bool>(size));
        
        for (int y = 0; y < size; y++)
        {
            for (int x = 0; x < size; x++)
            {
                result[y][x] = qr.getModule(x, y);
            }
        }
        
        return result;
    }
}

std::vector<std::vector<bool>> QRCodeUtils::generateFromText(const std::string& text, ErrorCorrectionLevel ecl)
{
    try
    {
        qrcodegen::QrCode::Ecc qrEcc = toQrCodegenEcc(ecl);
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(text.c_str(), qrEcc);
        return convertToMatrix(qr);
    }
    catch (const std::exception& e)
    {
        // Log error if needed
        return std::vector<std::vector<bool>>();
    }
}

std::vector<std::vector<bool>> QRCodeUtils::generateFromBinary(const std::vector<uint8_t>& data, ErrorCorrectionLevel ecl)
{
    try
    {
        qrcodegen::QrCode::Ecc qrEcc = toQrCodegenEcc(ecl);
        qrcodegen::QrCode qr = qrcodegen::QrCode::encodeBinary(data, qrEcc);
        return convertToMatrix(qr);
    }
    catch (const std::exception& e)
    {
        // Log error if needed
        return std::vector<std::vector<bool>>();
    }
}

std::string QRCodeUtils::toSvgString(const std::vector<std::vector<bool>>& qrCode, int border, const std::string& darkColor, const std::string& lightColor)
{
    if (qrCode.empty() || qrCode[0].empty())
    {
        return "";
    }

    int size = static_cast<int>(qrCode.size());
    
    // Validate that the matrix is square
    for (const auto& row : qrCode)
    {
        if (static_cast<int>(row.size()) != size)
        {
            return "";
        }
    }

    if (border < 0)
    {
        border = 0;
    }

    std::ostringstream sb;
    sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
    sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ";
    sb << (size + border * 2) << " " << (size + border * 2) << "\" stroke=\"none\">\n";
    sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"" << lightColor << "\"/>\n";
    sb << "\t<path d=\"";

    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            if (qrCode[y][x])
            {
                if (x != 0 || y != 0)
                {
                    sb << " ";
                }
                sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
            }
        }
    }

    sb << "\" fill=\"" << darkColor << "\"/>\n";
    sb << "</svg>\n";
    return sb.str();
}

int QRCodeUtils::getSize(const std::vector<std::vector<bool>>& qrCode)
{
    if (qrCode.empty())
    {
        return 0;
    }
    return static_cast<int>(qrCode.size());
}

bool QRCodeUtils::isDarkModule(const std::vector<std::vector<bool>>& qrCode, int x, int y)
{
    if (qrCode.empty() || y < 0 || y >= static_cast<int>(qrCode.size()))
    {
        return false;
    }
    if (x < 0 || x >= static_cast<int>(qrCode[y].size()))
    {
        return false;
    }
    return qrCode[y][x];
}

}
