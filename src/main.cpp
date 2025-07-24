#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/EditButtonBar.hpp>
#include "MoveMenu.hpp"

using namespace geode::prelude;

class $modify(EditUI, EditorUI) {
    static void onModify(auto& self) {
        (void)self.setHookPriority("EditorUI::init", Priority::LastPost); // trust issues
    }

    bool init(LevelEditorLayer* editorLayer) {
        if (!EditorUI::init(editorLayer)) return false;
        NodeIDs::provideFor(this); // trust issues
        moveMenu = MoveMenu::create(this);

        auto winSize = CCDirector::get()->getWinSize();

        auto pos = ccp( // mind my scuffed positioning i really dont care
            Loader::get()->isModLoaded("hjfod.betteredit") 
            ? (winSize.width / 2) - 35.0f 
            : (winSize.width / 2) - 27.5f, 60.0f
        ) * m_editButtonBar->getScale();
        moveMenu->setPosition(pos);
        moveMenu->setScale(m_editButtonBar->getScale() * 0.85); // i make good positioning decisions
        this->addChild(moveMenu);
        // set visible even whenever i set it every time an update happens just isnt working 
        // (hook prio or smth maybe not sure and idc enough to find out)
        // so fuck you this is easiest way to get the vanilla bar "out of the picture"
        m_editButtonBar->setScale(0.0f);
        // fine ill do it
        CCDirector::sharedDirector()->getScheduler()->scheduleSelector(schedule_selector(EditUI::updateMoveMenu), this, 0, false);
        return true; 
    }

    void updateMoveMenu(float dt) {
        if (!moveMenu) return;
        moveMenu->setVisible(m_editButtonBar->isVisible());
    }

    void moveObject(GameObject* p0, CCPoint p1) {
        if (!disableMoveObject) EditorUI::moveObject(p0, p1);
    }
};

class $modify(EditButtonBar) {
    void loadFromItems(CCArray* items, int r, int c, bool unkBool) {
        EditButtonBar::loadFromItems(items, r, c, unkBool);
        if (this->getID() == "edit-tab-bar") {
            if (moveMenu) moveMenu->update();
        }
    }
};

class $modify(LevelEditorLayer) {
    bool init(GJGameLevel* p0, bool p1) {
        // so we dont got a bad no good pointer whenever load from items trys to use it before the init hook
        moveMenu = nullptr;
        // bad better edit no cookie
        if (Loader::get()->isModLoaded("hjfod.betteredit")) Loader::get()->getLoadedMod("hjfod.betteredit")->setSettingValue<bool>("new-edit-menu", false);
        return LevelEditorLayer::init(p0, p1);
    }
};

$on_mod(Loaded) {
    listenForAllSettingChangesV3([] (std::shared_ptr<geode::SettingV3>){
        if (EditorUI::get() && moveMenu) moveMenu->update();
    });
}