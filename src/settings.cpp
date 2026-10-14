#include "main.hpp"
#include "modconfig.hpp"
#include "bsml/shared/BSML.hpp"
#include "custom-types/shared/register.hpp"
#include "HMUI/ViewController.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/UI/Toggle.hpp"
#include <array>
#include <vector>

namespace {
std::array<std::string_view,4> styles{"Off","Warm/Cold","True Random","Gradient"};
std::array<std::string_view,3> groups{"Standard","Isolated Event","Mayhem"};
std::vector<SafePtrUnity<UnityEngine::UI::Toggle>> masterToggles;
void SetEnabled(bool value) {
    getTechniqolorConfig().TechnicolorEnabled.SetValue(value);
    for(auto& toggle:masterToggles) if(toggle) toggle->SetIsOnWithoutNotify(value);
}
void AddMaster(UnityEngine::Transform* parent) {
    masterToggles.emplace_back(BSML::Lite::CreateToggle(parent,"Enable Techniqolor",getTechniqolorConfig().TechnicolorEnabled.GetValue(),SetEnabled)->toggle);
}
template<typename Value> void AddStyle(UnityEngine::Transform* parent,StringW title,Value& setting) {
    auto ptr=&setting;
    BSML::Lite::CreateDropdown(parent,title,std::string(styles[std::clamp(setting.GetValue(),0,3)]),std::span<std::string_view>(styles),[ptr](StringW value) {
        auto selected=static_cast<std::string>(value);
        for(int i=0;i<4;++i) if(selected==styles[i]) { ptr->SetValue(i); break; }
    });
}
void Settings(HMUI::ViewController* self,bool first,bool,bool) {
    if(!first) return;
    auto parent=BSML::Lite::CreateScrollableSettingsContainer(self->get_transform())->get_transform();
    auto& c=getTechniqolorConfig();
    BSML::Lite::CreateText(parent,"Techniqolor",5.f);
    AddMaster(parent);
    BSML::Lite::CreateText(parent,"Changes apply to the next level. Chroma / Noodle maps are skipped.",3.f);
    AddStyle(parent,"Lights",c.TechnicolorLightsStyle);
    BSML::Lite::CreateDropdown(parent,"Lights grouping",std::string(groups[std::clamp(c.TechnicolorLightsGrouping.GetValue(),0,2)]),std::span<std::string_view>(groups),[](StringW value) {
        auto selected=static_cast<std::string>(value);
        for(int i=0;i<3;++i) if(selected==groups[i]) { getTechniqolorConfig().TechnicolorLightsGrouping.SetValue(i); break; }
    });
    BSML::Lite::CreateIncrementSetting(parent,"Lights frequency / gradient speed",2,.05f,c.TechnicolorLightsFrequency.GetValue(),0.f,1.f,[](float v){getTechniqolorConfig().TechnicolorLightsFrequency.SetValue(v);});
    AddStyle(parent,"Notes",c.TechnicolorBlocksStyle);
    AddStyle(parent,"Sabers",c.TechnicolorSabersStyle);
    AddStyle(parent,"Walls",c.TechnicolorWallsStyle);
    AddStyle(parent,"Bombs",c.TechnicolorBombsStyle);
    BSML::Lite::CreateToggle(parent,"Desync left/right (PC behavior)",c.Desync.GetValue(),[](bool v){getTechniqolorConfig().Desync.SetValue(v);});
    BSML::Lite::CreateToggle(parent,"Disable gradient background",c.DisableGradientBackground.GetValue(),[](bool v){getTechniqolorConfig().DisableGradientBackground.SetValue(v);});
    BSML::Lite::CreateIncrementSetting(parent,"Color boost",2,.1f,c.ColorBoost.GetValue(),-1.f,2.f,[](float v){getTechniqolorConfig().ColorBoost.SetValue(v);});
    BSML::Lite::CreateText(parent,"Left hand overrides",4.f);
    BSML::Lite::CreateToggle(parent,"Separate left notes style",c.UseLeftBlocksStyle.GetValue(),[](bool v){getTechniqolorConfig().UseLeftBlocksStyle.SetValue(v);});
    AddStyle(parent,"Left notes",c.LeftTechnicolorBlocksStyle);
    BSML::Lite::CreateToggle(parent,"Separate left saber style",c.UseLeftSaberStyle.GetValue(),[](bool v){getTechniqolorConfig().UseLeftSaberStyle.SetValue(v);});
    AddStyle(parent,"Left saber",c.LeftTechnicolorSabersStyle);
}
}
void RegisterTechniqolorSettings() {
    custom_types::Register::AutoRegister(); BSML::Init();
    BSML::Register::RegisterSettingsMenu("Techniqolor",Settings,false);
    BSML::Register::RegisterGameplaySetupTab("Techniqolor",[](UnityEngine::GameObject* self,bool first) {
        if(!first) return;
        auto parent=BSML::Lite::CreateScrollableSettingsContainer(self->get_transform())->get_transform();
        AddMaster(parent);
        BSML::Lite::CreateText(parent,"Styles and brightness: Mod Settings > Techniqolor",3.f);
    },BSML::MenuType::Solo);
}
