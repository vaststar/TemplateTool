#include "QRCodeGenerator.h"
#include <ucf/Utilities/QRCodeUtils/QRCodeUtils.h>

QRCodeGenerator::QRCodeGenerator(QObject* parent)
    : QObject(parent)
    , m_text("")
    , m_errorLevel(1) // Medium
    , m_border(4)
    , m_darkColor("#000000")
    , m_lightColor("#FFFFFF")
    , m_svgData("")
    , m_valid(false)
{
}

QString QRCodeGenerator::text() const
{
    return m_text;
}

void QRCodeGenerator::setText(const QString& text)
{
    if (m_text != text)
    {
        m_text = text;
        emit textChanged();
        regenerate();
    }
}

int QRCodeGenerator::errorLevel() const
{
    return m_errorLevel;
}

void QRCodeGenerator::setErrorLevel(int level)
{
    if (m_errorLevel != level)
    {
        m_errorLevel = level;
        emit errorLevelChanged();
        regenerate();
    }
}

int QRCodeGenerator::border() const
{
    return m_border;
}

void QRCodeGenerator::setBorder(int border)
{
    if (m_border != border)
    {
        m_border = border;
        emit borderChanged();
        regenerate();
    }
}

QString QRCodeGenerator::darkColor() const
{
    return m_darkColor;
}

void QRCodeGenerator::setDarkColor(const QString& color)
{
    if (m_darkColor != color)
    {
        m_darkColor = color;
        emit darkColorChanged();
        regenerate();
    }
}

QString QRCodeGenerator::lightColor() const
{
    return m_lightColor;
}

void QRCodeGenerator::setLightColor(const QString& color)
{
    if (m_lightColor != color)
    {
        m_lightColor = color;
        emit lightColorChanged();
        regenerate();
    }
}

QString QRCodeGenerator::svgData() const
{
    return m_svgData;
}

bool QRCodeGenerator::valid() const
{
    return m_valid;
}

void QRCodeGenerator::regenerate()
{
    if (m_text.isEmpty())
    {
        m_svgData = "";
        m_valid = false;
        emit svgDataChanged();
        emit validChanged();
        return;
    }

    // Convert error level enum to QRCodeUtils error correction level
    ucf::Utilities::QRCodeUtils::ErrorCorrectionLevel ecl;
    switch (m_errorLevel)
    {
        case Low:
            ecl = ucf::Utilities::QRCodeUtils::ErrorCorrectionLevel::Low;
            break;
        case Medium:
            ecl = ucf::Utilities::QRCodeUtils::ErrorCorrectionLevel::Medium;
            break;
        case Quartile:
            ecl = ucf::Utilities::QRCodeUtils::ErrorCorrectionLevel::Quartile;
            break;
        case High:
            ecl = ucf::Utilities::QRCodeUtils::ErrorCorrectionLevel::High;
            break;
        default:
            ecl = ucf::Utilities::QRCodeUtils::ErrorCorrectionLevel::Medium;
            break;
    }

    // Generate QR code matrix
    auto qrMatrix = ucf::Utilities::QRCodeUtils::generateFromText(m_text.toStdString(), ecl);

    if (qrMatrix.empty())
    {
        m_svgData = "";
        m_valid = false;
        emit svgDataChanged();
        emit validChanged();
        return;
    }

    // Convert to SVG
    std::string svg = ucf::Utilities::QRCodeUtils::toSvgString(
        qrMatrix,
        m_border,
        m_darkColor.toStdString(),
        m_lightColor.toStdString()
    );

    QString newSvgData = QString::fromStdString(svg);
    bool newValid = !newSvgData.isEmpty();

    if (m_svgData != newSvgData)
    {
        m_svgData = newSvgData;
        emit svgDataChanged();
    }

    if (m_valid != newValid)
    {
        m_valid = newValid;
        emit validChanged();
    }
}
