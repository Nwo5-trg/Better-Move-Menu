#pragma once

class MoveMenu: public cocos2d::CCMenu {
private:
    EditorUI* m_editor;
    
    bool m_transformLock = false;
    bool m_dynamicArrowButtonPosition = false;
    float m_moveStep;
    float m_rotationStep;
    int m_pageLength;
    int m_page = 0;
    int m_pages;
    std::unordered_set<int> m_removedButtons;

    cocos2d::CCMenu* m_moveShortcutMenu = nullptr;
    cocos2d::CCMenu* m_rotateShortcutMenu = nullptr;
    cocos2d::CCMenu* m_buttonMenu = nullptr;
    geode::TextInput* m_moveInput = nullptr;
    geode::TextInput* m_rotationInput = nullptr;
    CCMenuItemSpriteExtra* m_nextButton = nullptr;
    CCMenuItemSpriteExtra* m_previousButton = nullptr;

    void onAction(cocos2d::CCObject* sender);
    void onToggleLock(cocos2d::CCObject* sender);
    void onSettings(cocos2d::CCObject* sender);
    void onMoveShortcut(cocos2d::CCObject* sender);
    void onRotateShortcut(cocos2d::CCObject* sender);
    void onNextPage(cocos2d::CCObject* sender);
    void onPreviousPage(cocos2d::CCObject* sender);
    void updateEditor();

public:
    static MoveMenu* create(EditorUI* editor);
    virtual bool init(EditorUI* editor);

    void update();
};

// idc if it works it works im not even namespacing fuck u ts pmo but im still doing it
inline bool disableMoveObject = false;
inline MoveMenu* moveMenu = nullptr;