#pragma once

#include <vector>

#include <QMetaType>

#include <commonhead/viewmodels/SideBarViewModel/SideBarModel.h>

Q_DECLARE_METATYPE(commonHead::viewModels::model::NavItemData)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::NavItemData>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::PageId)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::PageId>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::SubMenuItem)
Q_DECLARE_METATYPE(std::vector<commonHead::viewModels::model::SubMenuItem>)
Q_DECLARE_METATYPE(commonHead::viewModels::model::MenuActionId)
