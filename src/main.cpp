#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/Bindings.hpp>
#include <Geode/ui/GeodeUI.hpp>
#include <Geode/loader/Setting.hpp>

using namespace geode::prelude;

struct DeathCounterState {
    inline static int p1Deaths = 0;
    inline static int p2Deaths = 0;
    inline static bool p1DeathCounted = false;
    inline static bool p2DeathCounted = false;
};

// OK PLEASE IGNORE THAT P1 COUNTER IS M_PLAYER2
// I KNOW THAT LOOKS WEIRD BUT SOMEHOW IT WORKS
class $modify(PlayerObject) {
    void playerDestroyed(bool p0) {
        PlayerObject::playerDestroyed(p0);
        
        if (p0) {
            if (auto* pl = PlayLayer::get()) {
                if (this == pl->m_player2 && !DeathCounterState::p1DeathCounted) {
                    DeathCounterState::p1Deaths++;
                    DeathCounterState::p1DeathCounted = true;
                }
                else if (this == pl->m_player1 && !DeathCounterState::p2DeathCounted) {
                    DeathCounterState::p2Deaths++;
                    DeathCounterState::p2DeathCounted = true;
                }
            }
        }
    }
};

class $modify(MyPlayLayer, PlayLayer) {
    struct Fields {
        CCLabelBMFont* p1Counter = nullptr;
        CCLabelBMFont* p2Counter = nullptr;
        std::string namePlayer1;
        std::string namePlayer2;
        std::unique_ptr<geode::EventListener<geode::SettingChangedFilterV3>> settingListener;
    };

    bool init(GJGameLevel* level, bool p1, bool p2) {
        if (!PlayLayer::init(level, p1, p2)) return false;

        DeathCounterState::p1Deaths = 0;
        DeathCounterState::p2Deaths = 0;

        DeathCounterState::p1DeathCounted = false;
        DeathCounterState::p2DeathCounted = false;

        m_fields->namePlayer1 = Mod::get()->getSettingValue<std::string>("namePlayer1");
        m_fields->namePlayer2 = Mod::get()->getSettingValue<std::string>("namePlayer2");

        auto color1 = Mod::get()->getSettingValue<ccColor4B>("colorPlayer1");
        auto color2 = Mod::get()->getSettingValue<ccColor4B>("colorPlayer2");

        // can someone explain why is this yellow? well idc it works
        auto winSize = CCDirector::get()->getWinSize();
        float x1 = Mod::get()->getSettingValue<float>("XpositionPlayer1") * winSize.width;
        float y1 = Mod::get()->getSettingValue<float>("YpositionPlayer1") * winSize.height;
        float x2 = Mod::get()->getSettingValue<float>("XpositionPlayer2") * winSize.width;
        float y2 = Mod::get()->getSettingValue<float>("YpositionPlayer2") * winSize.height;

        m_fields->p1Counter = CCLabelBMFont::create(
            fmt::format("{}: 0", m_fields->namePlayer1).c_str(), "bigFont.fnt"
        );
        m_fields->p1Counter->setPosition(x1, y1);
        m_fields->p1Counter->setScale(0.5f);
        m_fields->p1Counter->setColor(ccColor3B{color1.r, color1.g, color1.b});
        m_fields->p1Counter->setOpacity(color1.a);
        this->addChild(m_fields->p1Counter);

        m_fields->p2Counter = CCLabelBMFont::create(
            fmt::format("{}: 0", m_fields->namePlayer2).c_str(), "bigFont.fnt"
        );
        m_fields->p2Counter->setPosition(x2, y2);
        m_fields->p2Counter->setScale(0.5f);
        m_fields->p2Counter->setColor(ccColor3B{color2.r, color2.g, color2.b});
        m_fields->p2Counter->setOpacity(color2.a);
        m_fields->p2Counter->setVisible(m_player2 != nullptr);
        this->addChild(m_fields->p2Counter);

        // settings changes thingy
        m_fields->settingListener.reset(
            geode::listenForAllSettingChanges(
                [this](std::shared_ptr<geode::SettingV3> setting) {
                    if (
                        setting->getKey() == "showCounter" ||
                        setting->getKey() == "namePlayer1" ||
                        setting->getKey() == "namePlayer2" ||
                        setting->getKey() == "colorPlayer1" ||
                        setting->getKey() == "colorPlayer2" ||
                        setting->getKey() == "XpositionPlayer1" ||
                        setting->getKey() == "YpositionPlayer1" ||
                        setting->getKey() == "XpositionPlayer2" ||
                        setting->getKey() == "YpositionPlayer2"
                    ) {
                        this->applySettings();
                    }
                },
                Mod::get()
            )
        );

        this->applySettings();
        return true;
    }

    void applySettings() {
        m_fields->namePlayer1 = Mod::get()->getSettingValue<std::string>("namePlayer1");
        m_fields->namePlayer2 = Mod::get()->getSettingValue<std::string>("namePlayer2");

        auto color1 = Mod::get()->getSettingValue<ccColor4B>("colorPlayer1");
        auto color2 = Mod::get()->getSettingValue<ccColor4B>("colorPlayer2");

        // yeah, there is a warning here but idk
        auto winSize = CCDirector::get()->getWinSize();
        float x1 = Mod::get()->getSettingValue<float>("XpositionPlayer1") * winSize.width;
        float y1 = Mod::get()->getSettingValue<float>("YpositionPlayer1") * winSize.height;
        float x2 = Mod::get()->getSettingValue<float>("XpositionPlayer2") * winSize.width;
        float y2 = Mod::get()->getSettingValue<float>("YpositionPlayer2") * winSize.height;

        if (m_fields->p1Counter) {
            m_fields->p1Counter->setString(fmt::format("{}: {}", m_fields->namePlayer1, DeathCounterState::p1Deaths).c_str());
            m_fields->p1Counter->setColor(ccColor3B{color1.r, color1.g, color1.b});
            m_fields->p1Counter->setOpacity(color1.a);
            m_fields->p1Counter->setPosition(x1, y1);
        }
        if (m_fields->p2Counter) {
            m_fields->p2Counter->setString(fmt::format("{}: {}", m_fields->namePlayer2, DeathCounterState::p2Deaths).c_str());
            m_fields->p2Counter->setColor(ccColor3B{color2.r, color2.g, color2.b});
            m_fields->p2Counter->setOpacity(color2.a);
            m_fields->p2Counter->setPosition(x2, y2);
        }
        this->updateCountersVisibility();
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        DeathCounterState::p1DeathCounted = false;
        DeathCounterState::p2DeathCounted = false;
        
        if (m_fields->p1Counter) {
            m_fields->p1Counter->setString(
                fmt::format("{}: {}", m_fields->namePlayer1, DeathCounterState::p1Deaths).c_str()
            );
        }
        if (m_fields->p2Counter) {
            m_fields->p2Counter->setString(
                fmt::format("{}: {}", m_fields->namePlayer2, DeathCounterState::p2Deaths).c_str()
            );
        }
    }

    void updateCountersVisibility() {
        bool visible = Mod::get()->getSettingValue<bool>("showCounter");
        if (m_fields->p1Counter) {
            m_fields->p1Counter->setVisible(visible);
        }
        if (m_fields->p2Counter) {
            m_fields->p2Counter->setVisible(visible && m_player2 != nullptr);
        }
    }

    void update(float dt) {
        PlayLayer::update(dt);
        if (m_fields->p2Counter) {
            bool visible = Mod::get()->getSettingValue<bool>("showCounter");
            m_fields->p2Counter->setVisible(visible && m_player2 != nullptr);
        }
    }


};

class $modify(MyPauseLayer, PauseLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* toggleButton = nullptr;
    };

    void customSetup() {
        PauseLayer::customSetup();
        
        // this button was previously a toggle button LOL
        auto toggleBtnSprite = CCSprite::create("erbotonsito.png"_spr);
        toggleBtnSprite->setScale(0.7f);
        
        m_fields->toggleButton = CCMenuItemSpriteExtra::create(
            toggleBtnSprite,
            this,
            menu_selector(MyPauseLayer::onToggleCounters)
        );
        
        // put the button in the right menu
        if (auto menu = this->getChildByID("right-button-menu")) {
            menu->addChild(m_fields->toggleButton);
            menu->updateLayout();
        }
    }

    void onToggleCounters(CCObject* sender) {
        // open the mod settings yaaaayyyy
        geode::openSettingsPopup(geode::Mod::get(), false);
    }
};