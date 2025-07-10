#include "MoveMenu.hpp"
#include "Utils.hpp"
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

MoveMenu* MoveMenu::create(EditorUI* editor) { // we copying geodes source code with this one :fire:
    auto ret = new MoveMenu();
    if (ret->init(editor)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MoveMenu::init(EditorUI* editor) {
    if (!CCMenu::init()) return false;
    this->setZOrder(1);
    this->setAnchorPoint({0.0f, 0.0f});

    auto mod = Mod::get();
    m_editor = editor;

    m_moveStep = mod->getSettingValue<double>("default-move");
    m_rotationStep = mod->getSettingValue<double>("default-rotation");

    std::array<CCPoint, 6> buttonPositions = {
        ccp(5.0f, 15.0f), ccp(35.0f, 0.0f), ccp(5.0f, -15.0f),
        ccp(-25.0f, 0.0f), ccp(-75.0f, 0.0f), ccp(-110.0f, 0.0f)
    };

    for (int i = 0; i < 6; i++) {
        auto button = CCMenuItemSpriteExtra::create( // how in the hell am i even supposed to format this
            ButtonSprite::create(CCSprite::create(
                    ("nwo5.better_move_menu/buttontexture" + std::to_string(i) + ".png").c_str()), 
                25.0f, true, 25.0f, "GJ_button_01.png", 1),
            this, menu_selector(MoveMenu::onAction)
        );
        button->setScale(i < 4 ? 0.65f : 0.75f);
        button->m_baseScale = i < 4 ? 0.65f : 0.75f;
        button->setPosition(buttonPositions[i]);
        button->setTag(i);
        this->addChild(button);
    }
    
    auto moveInput = TextInput::create(50.0f, "15");
    moveInput->setString(ftofstr(m_moveStep, 8));
    moveInput->setPosition(85.0f, 0.0f);
    moveInput->setFilter("1234567890.");
    moveInput->setCallback([this] (const std::string& input) {
        if (!input.empty()) m_moveStep = std::strtof(input.c_str(), nullptr);
    });
    this->addChild(moveInput);
    m_moveInput = moveInput;

    auto rotationInput = TextInput::create(60.0f, "45");
    rotationInput->setString(ftofstr(m_rotationStep, 8));
    rotationInput->setScale(0.85f);
    rotationInput->setPosition(-165.0f, 15.0f);
    rotationInput->setFilter("1234567890.");
    rotationInput->setCallback([this] (const std::string& input) {
        if (!input.empty()) m_rotationStep = std::strtof(input.c_str(), nullptr);
    });
    this->addChild(rotationInput);
    m_rotationInput = rotationInput;

    auto buttonMenu = CCMenu::create();
    buttonMenu->setPosition(0.0f, -50.0f);
    this->addChild(buttonMenu);
    m_buttonMenu = buttonMenu;

    auto nextButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"), 
        this, menu_selector(MoveMenu::onNextPage)
    );
    nextButton->setScale(0.55f);
    nextButton->m_baseScale = 0.55f;
    nextButton->setRotationY(180.0f);
    this->addChild(nextButton);
    m_nextButton = nextButton;

    auto previousButton = CCMenuItemSpriteExtra::create(
        CCSprite::createWithSpriteFrameName("GJ_arrow_02_001.png"), 
        this, menu_selector(MoveMenu::onPreviousPage)
    );
    previousButton->setScale(0.55f);
    previousButton->m_baseScale = 0.55f;
    this->addChild(previousButton);
    m_previousButton = previousButton;

    auto moveShortcutMenu = CCMenu::create();
    moveShortcutMenu->setPosition(165.0f, -0.0f);
    this->addChild(moveShortcutMenu);
    m_moveShortcutMenu = moveShortcutMenu;

    auto rotateShortcutMenu = CCMenu::create();
    rotateShortcutMenu->setPosition(-180.0f, -20.0f);
    this->addChild(rotateShortcutMenu);
    m_rotateShortcutMenu = rotateShortcutMenu;

    auto lockButton = CCMenuItemToggler::create(
        CCSprite::createWithSpriteFrameName("warpLockOffBtn_001.png"),
        CCSprite::createWithSpriteFrameName("warpLockOnBtn_001.png"),
        this, menu_selector(MoveMenu::onToggleLock)
    );
    lockButton->setPosition(-210.0f, 15.0f);
    this->addChild(lockButton);

    if (mod->getSettingValue<bool>("show-settings-button")) {
        auto settingsButton = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("accountBtn_settings_001.png"),
            this, menu_selector(MoveMenu::onSettings)
        );
        settingsButton->setScale(0.25f);
        settingsButton->m_baseScale = 0.25f;
        settingsButton->setPosition(210.0f, -55.0f);
        this->addChild(settingsButton);
    }

    update();

    return true;
}

void MoveMenu::update() {
    auto mod = Mod::get();

    m_pageLength = mod->getSettingValue<int64_t>("page-length");
    m_removedButtons = parseIntStringToSet(mod->getSettingValue<std::string>("removed-buttons"));

    m_moveShortcutMenu->removeAllChildrenWithCleanup(true);
    m_rotateShortcutMenu->removeAllChildrenWithCleanup(true);

    { // move normal buttons over
        for (int i = 0; i < m_editor->m_editButtonBar->m_buttonArray->count(); i++) {
            auto button = static_cast<CCMenuItemSpriteExtra*>(m_editor->m_editButtonBar->m_buttonArray->objectAtIndex(i));
            button->removeFromParentAndCleanup(false); // hopefully this is the right thing to be doing
            auto id = button->getID();
            if (!id.empty() && m_buttonMenu->getChildByID(id)) continue;
            if (id.starts_with("move-") || m_removedButtons.contains(i - 20)) { // -20 cuz move buttons
                button->removeMeAndCleanup(); // yk what nvm i dont trust it
                continue;
            }
            button->setScale(0.525f);
            button->m_baseScale = 0.525f;
            m_buttonMenu->addChild(button);
        }
        m_editor->m_editButtonBar->m_buttonArray->removeAllObjects();
        auto buttons = m_buttonMenu->getChildren();

        m_pages = std::ceil((buttons->count() + m_pageLength - 1) / m_pageLength);
        if (m_page >= m_pages) m_page = 0;

        for (int i = 0; i < m_pages; i++) {
            int start = i * m_pageLength;
            int end = std::min((int)buttons->count(), start + m_pageLength);
            int count = end - start;
            float total = 25.0f * (count - 1);
            float startX = total / -2;

            for (int j = 0; j < count; j++) {
                auto node = static_cast<CCNode*>(buttons->objectAtIndex(start + j));
                float x = startX + j * 25.0f;
                node->setPosition(x, 0.0f);
                bool visible = i == m_page;
                node->setVisible(visible);

                if (visible) {
                    if (j == 0) m_previousButton->setPosition(ccp(x - 25.0f, 0.0f) + m_buttonMenu->getPosition());
                    if (j == count - 1) m_nextButton->setPosition(ccp(x + 25.0f, 0.0f) + m_buttonMenu->getPosition());
                }
            }
        }
        m_nextButton->setVisible(m_pageLength < buttons->count());
        m_previousButton->setVisible(m_pageLength < buttons->count());
    }

    std::array<float, 9> shortcutValues;
    for (int i = 0; i < 9; i++) shortcutValues[i] = mod->getSettingValue<double>("shortcut-value-" + std::to_string(i));
    std::array<std::string, 9> shortcutLabels;
    for (int i = 0; i < 9; i++) shortcutLabels[i] = mod->getSettingValue<std::string>("shortcut-label-" + std::to_string(i));
    std::array<CCPoint, 9> shortcutPositions = {
        ccp(-30.0f, 15.0f), ccp(0.0f, 15.0f), ccp(30.0f, 15.0f),
        ccp(-30.0f, -15.0f), ccp(0.0f, -15.0f), ccp(30.0f, -15.0f),
        ccp(-30.0f, 0.0f), ccp(0.0f, 0.0f), ccp(30.0f, 00.0f),
    };

    for (int i = 0; i < 9; i++) {
        auto button = CCMenuItemSpriteExtra::create(CircleButtonSprite::create(
        CCLabelBMFont::create(
            shortcutLabels[i].c_str(), "bigFont.fnt")), 
            this, i < 6 ? menu_selector(MoveMenu::onMoveShortcut) 
            : menu_selector(MoveMenu::onRotateShortcut)
        );
        button->setScale(0.6f);
        button->m_baseScale = 0.6f;
        button->setPosition(shortcutPositions[i]);
        auto valueNode = CCNode::create();
        valueNode->setID(ftofstr(shortcutValues[i], 8));
        button->addChild(valueNode);
        if (i < 6) m_moveShortcutMenu->addChild(button);
        else m_rotateShortcutMenu->addChild(button);
    }
}

void MoveMenu::onNextPage(CCObject* sender) {
    if (++m_page >= m_pages) m_page = 0;
    update();
}

void MoveMenu::onPreviousPage(CCObject* sender) {
    if (--m_page <= -1) m_page = m_pages - 1;
    update();
}

void MoveMenu::onAction(CCObject* sender) {
    int type = static_cast<CCNode*>(sender)->getTag();
    auto objs = m_editor->getSelectedObjects();

    switch (type) { // idc if this could be refactored or wtv this is easier
        case 0: {
            for (auto obj : CCArrayExt<GameObject*>(objs)) m_editor->moveObject(obj, {0.0f, m_moveStep});
            updateEditor(); break;
        }
        case 1: {
            for (auto obj : CCArrayExt<GameObject*>(objs)) m_editor->moveObject(obj, {m_moveStep, 0.0f});
            updateEditor(); break;
        }
        case 2: {
            for (auto obj : CCArrayExt<GameObject*>(objs)) m_editor->moveObject(obj, {0.0f, -m_moveStep});
            updateEditor(); break;
        }
        case 3: {
            for (auto obj : CCArrayExt<GameObject*>(objs)) m_editor->moveObject(obj, {-m_moveStep, 0.0f});
            updateEditor(); break;
        }
        case 4: {
            disableMoveObject = m_transformLock;
            m_editor->rotateObjects(objs, m_rotationStep, m_editor->getGroupCenter(objs, true));
            updateEditor(); break;
        }
        case 5: {
            disableMoveObject = m_transformLock;
            m_editor->rotateObjects(objs, -m_rotationStep, m_editor->getGroupCenter(objs, true));
            updateEditor(); break;
        }
    }
}

void MoveMenu::onMoveShortcut(CCObject* sender) {
    m_moveInput->setString(static_cast<CCNode*>(sender)->getChildByType<CCNode*>(1)->getID(), true);
}

void MoveMenu::onRotateShortcut(CCObject* sender) {
    m_rotationInput->setString(static_cast<CCNode*>(sender)->getChildByType<CCNode*>(1)->getID(), true);
}

void MoveMenu::onToggleLock(CCObject* sender) {
    m_transformLock = !m_transformLock;
}

void MoveMenu::onSettings(CCObject* sender) {
    geode::openSettingsPopup(Mod::get(), false);
}

void MoveMenu::updateEditor() {
    disableMoveObject = false;
    m_editor->updateButtons();
    m_editor->updateDeleteButtons();
    m_editor->updateObjectInfoLabel();
    if (m_editor->m_rotationControl->isVisible()) { // update rotation control idk an proper function to do it sooo
        m_editor->deactivateRotationControl();
        m_editor->activateRotationControl(nullptr);
    }
}