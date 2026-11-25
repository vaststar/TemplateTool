UIViewAppendBuildInputs(
    SOURCES
        include/ChatPage/ChatPageController.h
        src/ChatPageController.cpp
    QML_ALIAS_PREFIX PageViews/ChatPage
    QML_FILES
        qml/ChatPage.qml
        qml/ChatConversationList.qml
        qml/ChatThread.qml
        qml/ChatInviteDialog.qml
    PRIVATE_INCLUDE_DIRS
        include
        include/ChatPage
    PRIVATE_DEPENDENCIES
        ChatViewModelApi
)
