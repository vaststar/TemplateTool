#include <Constants/FontConstants.h>

FontConstants::FontConstants(QObject* parent)
    : QObject(parent)
{

}

QFont FontConstants::getBodySecondary() const
{
    return QFont("times",10, QFont::Bold);
}