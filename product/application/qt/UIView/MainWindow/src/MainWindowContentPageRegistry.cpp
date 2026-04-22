#include "MainWindow/include/MainWindowContentPageRegistry.h"

#include <commonHead/viewModels/SideBarViewModel/SideBarModel.h>

MainWindowContentPageRegistry::MainWindowContentPageRegistry(QObject* parent)
    : QObject(parent)
{
    using PageId = commonHead::viewModels::model::PageId;
    registerPage(static_cast<int>(PageId::Home),        QStringLiteral("UIView/PageViews/HomePage/qml/HomePage.qml"));
    registerPage(static_cast<int>(PageId::Contacts),    QStringLiteral("UIView/PageViews/ContactsPage/qml/ContactsPage.qml"));
    registerPage(static_cast<int>(PageId::Tasks),       QStringLiteral("UIView/PageViews/PlaceholderPage/qml/PlaceholderPage.qml"));
    registerPage(static_cast<int>(PageId::Credentials), QStringLiteral("UIView/PageViews/PlaceholderPage/qml/PlaceholderPage.qml"));
    registerPage(static_cast<int>(PageId::Toolbox),     QStringLiteral("UIView/PageViews/ToolsPage/common/qml/ToolsPage.qml"));
    registerPage(static_cast<int>(PageId::Settings),    QStringLiteral("UIView/PageViews/SettingsPage/qml/SettingsPage.qml"));
    registerPage(static_cast<int>(PageId::Help),        QStringLiteral("UIView/PageViews/PlaceholderPage/qml/PlaceholderPage.qml"));
}

void MainWindowContentPageRegistry::registerPage(int pageId, const QString& qmlSource)
{
    m_pages.append({pageId, QStringLiteral("qrc:/qt/qml/") + qmlSource});
}

QStringList MainWindowContentPageRegistry::entries() const
{
    QStringList list;
    list.reserve(m_pages.size());
    for (const auto& p : m_pages)
        list.append(p.second);
    return list;
}

int MainWindowContentPageRegistry::indexOfPage(int pageId) const
{
    for (int i = 0; i < m_pages.size(); ++i) {
        if (m_pages[i].first == pageId)
            return i;
    }
    return 0;
}
