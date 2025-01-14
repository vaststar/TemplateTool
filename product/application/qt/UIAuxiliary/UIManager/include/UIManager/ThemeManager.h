#pragma once

#include <memory>
#include <QObject>
#include <QString>
#include <QtQml>

#include <UIDataStruct/UIFontSet.h>
#include <UIDataStruct/UIColorSet.h>
#include <UIManager/UIManagerExport.h>

namespace UICore{
    class CoreApplication;
    class CoreQmlEngine;
}

namespace UIManager{
class UIManager_EXPORT ThemeManager final: public QObject
{
Q_OBJECT
QML_ELEMENT
public:
    explicit ThemeManager(UICore::CoreApplication* application, UICore::CoreQmlEngine* qmlEngine);
    ~ThemeManager();
public slots:
    void test();
    QColor getUIColor(UIData::UIColors::UIColorsEnum colorEnum, UIData::UIColors::UIColorState state);
    QFont getUIFont(UIData::UIFont::UIFontSize size, UIData::UIFont::UIFontWeight weight = UIData::UIFont::UIFontWeight::UIFontWeight_Normal, bool isItalic = false, UIData::UIFont::UIFontFamily family = UIData::UIFont::UIFontFamily::UIFontFamily_SegoeUI);
private:
    class Impl;
    std::unique_ptr<Impl> mImpl;
};
}
