#include <UIDataStruct/UIColorSet.h>

namespace UIData{

UIColorSet::UIColorSet(QObject *parent)
    : QObject(parent)
{

}

UIColorSet::UIColorSet(const QString& name, const QColor& normal)
    : name(name)
    , normal(normal)
{

}

UIColorSet::UIColorSet(const QString& name, const QColor& normal, const QString& hovered)
    : name(name)
    , normal(normal)
    , hovered(hovered)
{

}
}