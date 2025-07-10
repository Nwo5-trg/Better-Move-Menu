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
        moveMenu->setPosition(Loader::get()->isModLoaded("hjfod.betteredit") ? 292.0f : 285.0f, 60.0f); // mind my scuffed positioning i really dont care
        moveMenu->setScale(0.85f);
        this->addChild(moveMenu);
        // set visible even whenever i set it every time an update happens just isnt working 
        // (hook prio or smth maybe not sure and idc enough to find out)
        // so fuck you this is easiest way to get the vanilla bar "out of the picture"
        m_editButtonBar->setScale(0.0f);
        return true;
    }

    void draw() {
        EditorUI::draw();
        // from be server: "i should never touch editorui again, but it *technically* works"
        if (moveMenu) moveMenu->setVisible(m_editButtonBar->isVisible());
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