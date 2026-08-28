#pragma once

#include <memory>
#include <string>

#include <commonhead/viewmodels/ViewModelCore/IViewModel.h>
#include <commonhead/viewmodels/JsonToolViewModel/IJsonTreeModel.h>
#include <commonhead/viewmodels/JsonToolViewModel/JsonOperationTypes.h>

namespace commonHead::viewModels {

/**
 * @brief JSON 工具 ViewModel 接口
 *
 * 负责 JSON 格式化、校验和树形结构构建。
 */
class IJsonToolViewModel : public IViewModel
{
public:
    using IViewModel::IViewModel;
    IJsonToolViewModel(const IJsonToolViewModel&) = delete;
    IJsonToolViewModel(IJsonToolViewModel&&) = delete;
    IJsonToolViewModel& operator=(const IJsonToolViewModel&) = delete;
    IJsonToolViewModel& operator=(IJsonToolViewModel&&) = delete;
    ~IJsonToolViewModel() override = default;

public:
    [[nodiscard]] virtual JsonFormatResult format(
        const std::string& input,
        int indent = 2) const = 0;
    [[nodiscard]] virtual JsonFormatResult minify(const std::string& input) const = 0;
    [[nodiscard]] virtual JsonFormatResult validate(const std::string& input) const = 0;

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
};

} // namespace commonHead::viewModels
