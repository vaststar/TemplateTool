#pragma once

#include <memory>
#include <string>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/JsonTreeViewModel/IJsonTreeModel.h>

namespace commonHead::viewModels {

/**
 * @brief JSON 树 ViewModel 接口
 *
 * 负责将 JSON 字符串解析为树形结构，供 UI 层的 TreeView 展示。
 */
class COMMONHEAD_EXPORT IJsonTreeViewModel : public IViewModel
{
public:
    using IViewModel::IViewModel;
    IJsonTreeViewModel(const IJsonTreeViewModel&) = delete;
    IJsonTreeViewModel(IJsonTreeViewModel&&) = delete;
    IJsonTreeViewModel& operator=(const IJsonTreeViewModel&) = delete;
    IJsonTreeViewModel& operator=(IJsonTreeViewModel&&) = delete;
    virtual ~IJsonTreeViewModel() = default;

public:
    virtual std::string getViewModelName() const = 0;

    /**
     * @brief 解析 JSON 字符串并构建树
     * @param jsonStr JSON 字符串
     * @return 成功返回 JsonTreePtr，解析失败返回 nullptr
     */
    virtual model::JsonTreePtr parseJsonTree(const std::string& jsonStr) = 0;

    /**
     * @brief 获取解析错误信息（parseJsonTree 返回 nullptr 时可调用）
     */
    virtual std::string getLastError() const = 0;

    /**
     * @brief 获取节点的 JSON Path
     */
    virtual std::string getJsonPath(const model::JsonTreePtr& tree, const model::IJsonTreeNode* node) const = 0;

    /**
     * @brief 获取节点的子树 JSON 字符串
     */
    virtual std::string getSubTreeJson(const model::JsonTreePtr& tree, const model::IJsonTreeNode* node, int indent = 2) const = 0;

public:
    static std::shared_ptr<IJsonTreeViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
