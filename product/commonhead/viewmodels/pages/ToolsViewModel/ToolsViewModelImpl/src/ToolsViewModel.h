#pragma once

#include <memory>
#include <vector>

#include <commonhead/utilities/VMNotificationHelper/VMNotificationHelper.h>
#include <commonhead/viewmodels/ToolsViewModel/IToolsViewModel.h>

namespace commonHead {
class IResourceLoader;
}

namespace commonHead::viewModels {

// Internal implementation; construction is exposed through ToolsViewModelCreator.

class ToolsViewModel : public virtual IToolsViewModel,
                       public virtual commonHead::utilities::VMNotificationHelper<IToolsViewModelCallback>
{
public:
    explicit ToolsViewModel(commonHead::ICommonHeadFrameworkWptr framework);
    ~ToolsViewModel() override = default;

    std::string getViewModelName() const override;

    //========================================
    // 工具树导航
    //========================================
    model::ToolsTreePtr getToolsTree() const override;
    bool isToolsTreeReady() const override;
    void selectNode(const std::string& nodeId) override;
    void reloadTree() override;

protected:
    void init() override;

private:
    [[nodiscard]] std::shared_ptr<commonHead::IResourceLoader> lockResourceLoader() const;
    [[nodiscard]] bool buildToolsTree();
    [[nodiscard]] bool refreshTreeNodeData();
    // DFS-collect the full node list (excluding the virtual root), in pre-order.
    std::vector<model::ToolNodeData> snapshotAllNodes() const;

private:
    model::ToolsTreePtr m_toolsTree;
    bool m_ready = false;
};

} // namespace commonHead::viewModels
