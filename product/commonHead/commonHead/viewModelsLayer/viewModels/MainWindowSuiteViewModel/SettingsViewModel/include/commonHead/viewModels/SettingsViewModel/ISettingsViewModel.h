#pragma once

#include <memory>
#include <string>
#include <vector>

#include <commonHead/CommonHeadCommonFile/CommonHeadExport.h>
#include <commonHead/commonHeadUtils/VMNotificationHelper/IVMNotificationHelper.h>
#include <commonHead/viewModels/IViewModel/IViewModel.h>
#include <commonHead/viewModels/SettingsViewModel/ISettingsModel.h>

namespace commonHead::viewModels {

/**
 * @brief Settings ViewModel callback interface.
 *
 * Modeled after IContactListViewModelCallback: VM emits value-typed batch
 * notifications so the UI layer can maintain its own mirror without ever
 * touching the VM's internal tree object.
 */
class COMMONHEAD_EXPORT ISettingsViewModelCallback
{
public:
    ISettingsViewModelCallback() = default;
    ISettingsViewModelCallback(const ISettingsViewModelCallback&) = delete;
    ISettingsViewModelCallback(ISettingsViewModelCallback&&) = delete;
    ISettingsViewModelCallback& operator=(const ISettingsViewModelCallback&) = delete;
    ISettingsViewModelCallback& operator=(ISettingsViewModelCallback&&) = delete;
    virtual ~ISettingsViewModelCallback() = default;

public:
    /**
     * @brief Fired once when the initial tree has been built. Late subscribers
     *        won't get this; they should consult isSettingsTreeReady() and
     *        snapshot via getSettingsTree().
     */
    virtual void onSettingsTreeReady() {}

    /**
     * @brief A batch of new nodes was added. Each NodeData carries its parentId.
     */
    virtual void onSettingsNodesAdded(const std::vector<model::SettingsNodeData>& /*nodes*/) {}

    /**
     * @brief A batch of existing nodes had their data refreshed in-place
     *        (parent and id are stable). E.g. language change refreshes titles.
     */
    virtual void onSettingsNodesUpdated(const std::vector<model::SettingsNodeData>& /*nodes*/) {}

    /**
     * @brief A batch of nodes was removed.
     */
    virtual void onSettingsNodesRemoved(const std::vector<std::string>& /*nodeIds*/) {}
};

class COMMONHEAD_EXPORT ISettingsViewModel
    : public IViewModel
    , public virtual commonHead::utilities::IVMNotificationHelper<ISettingsViewModelCallback>
{
public:
    using IViewModel::IViewModel;
    ISettingsViewModel(const ISettingsViewModel&) = delete;
    ISettingsViewModel(ISettingsViewModel&&) = delete;
    ISettingsViewModel& operator=(const ISettingsViewModel&) = delete;
    ISettingsViewModel& operator=(ISettingsViewModel&&) = delete;
    virtual ~ISettingsViewModel() = default;

public:
    virtual std::string getViewModelName() const = 0;

    /**
     * @brief Snapshot accessor; kept so callers (controller init, late
     *        subscribers) can grab the current tree without waiting for
     *        the onSettingsTreeReady callback.
     */
    virtual model::SettingsTreePtr getSettingsTree() const = 0;

    /**
     * @brief True once the tree has been populated. onSettingsTreeReady will
     *        not be re-fired for late subscribers.
     */
    [[nodiscard]] virtual bool isSettingsTreeReady() const = 0;

    /**
     * @brief Notify VM that the user selected a node. VM owns no selection state;
     *        this hook exists purely so metrics/telemetry can be added later.
     */
    virtual void selectNode(const std::string& nodeId) = 0;

    /**
     * @brief Refresh localized strings after a language switch. VM will emit
     *        onSettingsNodesUpdated with the full node list.
     */
    virtual void reloadTree() = 0;

public:
    static std::shared_ptr<ISettingsViewModel> createInstance(
        commonHead::ICommonHeadFrameworkWptr commonHeadFramework);
};

} // namespace commonHead::viewModels
