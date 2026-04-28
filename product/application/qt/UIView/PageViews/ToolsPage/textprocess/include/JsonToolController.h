#pragma once

#include <QObject>
#include <QtQml>
#include "UIViewCommon/UIViewBase/include/UIViewController.h"
#include "PageViews/ToolsPage/textprocess/include/JsonTreeItemModel.h"

namespace commonHead::viewModels {
    class IToolsViewModel;
    class IJsonTreeViewModel;
}

namespace commonHead::viewModels::model {
    class IJsonTree;
    class IJsonTreeNode;
}

class JsonToolController : public UIViewController
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QString inputText READ getInputText WRITE setInputText NOTIFY inputTextChanged)
    Q_PROPERTY(QString outputText READ getOutputText NOTIFY outputTextChanged)
    Q_PROPERTY(QString errorMessage READ getErrorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(int indentSize READ getIndentSize WRITE setIndentSize NOTIFY indentSizeChanged)
    Q_PROPERTY(JsonTreeItemModel* treeModel READ getTreeModel CONSTANT)
    Q_PROPERTY(bool treeViewMode READ isTreeViewMode NOTIFY treeViewModeChanged)

public:
    explicit JsonToolController(QObject* parent = nullptr);

    QString getInputText() const;
    void setInputText(const QString& text);
    QString getOutputText() const;
    QString getErrorMessage() const;
    int getIndentSize() const;
    void setIndentSize(int indent);
    JsonTreeItemModel* getTreeModel() const;
    bool isTreeViewMode() const;

    Q_INVOKABLE void format();
    Q_INVOKABLE void minify();
    Q_INVOKABLE void validate();
    Q_INVOKABLE void copyOutput();
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void parseTree();
    Q_INVOKABLE void copyNodeValue(int row, int depth);
    Q_INVOKABLE QString getNodeJsonPath(int row, int depth);
    Q_INVOKABLE void copySubTree(int row, int depth);

protected:
    void init() override;

signals:
    void inputTextChanged();
    void outputTextChanged();
    void errorMessageChanged();
    void indentSizeChanged();
    void treeViewModeChanged();

private:
    std::shared_ptr<commonHead::viewModels::IToolsViewModel> m_toolsViewModel;
    std::shared_ptr<commonHead::viewModels::IJsonTreeViewModel> m_jsonTreeViewModel;
    JsonTreeItemModel* m_treeModel = nullptr;
    std::shared_ptr<commonHead::viewModels::model::IJsonTree> m_currentTree;
    QString m_inputText;
    QString m_outputText;
    QString m_errorMessage;
    int m_indentSize = 2;
    bool m_treeViewMode = false;
};
