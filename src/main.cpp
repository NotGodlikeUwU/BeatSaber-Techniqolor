#include "main.hpp"
#include "modconfig.hpp"
#include "unity_ref.hpp"
#include "GlobalNamespace/GameplayCoreInstaller.hpp"
#include "GlobalNamespace/GameplayCoreSceneSetupData.hpp"
#include "GlobalNamespace/BeatmapCharacteristicSO.hpp"
#include "GlobalNamespace/AudioTimeSyncController.hpp"
#include "GlobalNamespace/ColorNoteVisuals.hpp"
#include "GlobalNamespace/NoteControllerBase.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteCutInfo.hpp"
#include "GlobalNamespace/SliderController.hpp"
#include "GlobalNamespace/SliderData.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/BombNoteController.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/StretchableObstacle.hpp"
#include "GlobalNamespace/MaterialPropertyBlockController.hpp"
#include "GlobalNamespace/SaberModelController.hpp"
#include "GlobalNamespace/Saber.hpp"
#include "GlobalNamespace/SaberTrail.hpp"
#include "GlobalNamespace/SetSaberGlowColor.hpp"
#include "GlobalNamespace/SetSaberFakeGlowColor.hpp"
#include "GlobalNamespace/TubeBloomPrePassLight.hpp"
#include "GlobalNamespace/ColorManager.hpp"
#include "GlobalNamespace/ColorType.hpp"
#include "GlobalNamespace/SaberType.hpp"
#include "GlobalNamespace/EnvironmentColorType.hpp"
#include "GlobalNamespace/LightSwitchEventEffect.hpp"
#include "GlobalNamespace/LightColorGroupEffect.hpp"
#include "GlobalNamespace/LightColorBeatmapEventData.hpp"
#include "GlobalNamespace/BasicBeatmapEventData.hpp"
#include "GlobalNamespace/LightWithIdManager.hpp"
#include "GlobalNamespace/ILightWithId.hpp"
#include "GlobalNamespace/ParticleSystemEventEffect.hpp"
#include "GlobalNamespace/BloomPrePassBackgroundColorsGradientFromColorSchemeColors.hpp"
#include "GlobalNamespace/BloomPrePassBackgroundColorsGradient.hpp"
#include "UnityEngine/MaterialPropertyBlock.hpp"
#include "UnityEngine/Shader.hpp"
#include "UnityEngine/ParticleSystem.hpp"
#include "UnityEngine/Color32.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Time.hpp"
#include "UnityEngine/Application.hpp"
#include "songcore/shared/SongLoader/CustomBeatmapLevel.hpp"
#include "System/Collections/Generic/List_1.hpp"
#include <optional>
#include <unordered_map>
#include <vector>

using namespace GlobalNamespace;
using UnityEngine::Color;
using techni::Style;
using techni::Grouping;
namespace {
modloader::ModInfo modInfo{MOD_ID, VERSION, 0};
techni::Options options;
techni::Random techniRandom;
techni::Random rainbowRandom(739);
bool inLevel=false, recoloringSaber=false;
AudioTimeSyncController* clockOwner=nullptr;
float now=0;
int colorId=0, simpleColorId=0;
std::optional<techni::Color> saberOverride, globalLightHue;
struct Note { UnityRef<ColorNoteVisuals> visual; bool left; Style style; };
struct Bomb { UnityRef<BombNoteController> controller; std::vector<UnityRef<MaterialPropertyBlockController>> blocks; Color original; };
struct Wall { UnityRef<ObstacleController> controller; Color original; };
struct SaberModel { UnityRef<SaberModelController> model; bool left; Color trailTint; techni::Color from, to; float lastTime; };
struct LightState { Color original; bool warm=false; std::optional<techni::Color> hue; std::unordered_map<ILightWithId*,techni::Color> individual; };
struct LightManager { UnityRef<LightWithIdManager> manager; std::unordered_map<int,LightState> groups; };
struct ParticleEffect { UnityRef<ParticleSystemEventEffect> effect; bool warm; std::optional<techni::Color> hue; };
std::vector<Note> notes;
std::vector<Bomb> bombs;
std::vector<Wall> walls;
std::vector<SaberModel> sabers;
std::unordered_map<LightWithIdManager*,LightManager> lightManagers;
std::unordered_map<ParticleSystemEventEffect*,ParticleEffect> particleEffects;
std::vector<UnityRef<BloomPrePassBackgroundColorsGradientFromColorSchemeColors>> backgrounds;
Color ToUnity(techni::Color c) { return {c.r,c.g,c.b,c.a}; }
techni::Color ToNative(Color c) { return {c.r,c.g,c.b,c.a}; }
techni::Color ChooseFlat(bool warm,Style style) { return techni::flat(warm,style,style==Style::PureRandom ? rainbowRandom : techniRandom); }
bool Running() { return inLevel && options.enabled; }
template <typename T> bool Visible(T* object) { return object && object->get_gameObject()->get_activeInHierarchy(); }
void Reset() {
    inLevel=false; clockOwner=nullptr; recoloringSaber=false; saberOverride.reset(); globalLightHue.reset();
    notes.clear(); bombs.clear(); walls.clear(); sabers.clear(); lightManagers.clear(); backgrounds.clear(); particleEffects.clear();
    techniRandom=techni::Random(400);
    rainbowRandom=techni::Random(739);
}
void SetBlock(MaterialPropertyBlockController* controller, Color color, bool simple=false) {
    if (!controller) return;
    auto block=controller->get_materialPropertyBlock();
    if (!block) return;
    block->SetColor(colorId,color);
    if (simple) block->SetColor(simpleColorId,color);
    controller->ApplyChanges();
}
void ColorNote(ColorNoteVisuals* visual, techni::Color color) {
    auto c=ToUnity(color); visual->_noteColor=c;
    if (auto blocks=visual->_materialPropertyBlockControllers) for (auto block:blocks) {
        if(!block || !block->get_materialPropertyBlock()) continue;
        c.a=block->get_materialPropertyBlock()->GetColor(colorId).a;
        SetBlock(block.ptr(),c);
    }
}
void ColorBomb(Bomb& bomb, techni::Color color) {
    auto c=ToUnity(color); c.a=bomb.original.a;
    for (auto& block:bomb.blocks) if(block) SetBlock(block.ptr(),c,true);
}
void ColorWall(ObstacleController* wall, techni::Color color, float alpha) {
    if (!wall->_stretchableObstacle) return;
    auto c=ToUnity(color); c.a=alpha; wall->_color=c;
    wall->_stretchableObstacle->SetAllProperties(wall->get_width(),wall->get_height(),wall->get_length(),c,wall->get_manualUvOffset());
}
techni::Color SaberColor(SaberModel& saber) {
    auto style=options.saberStyle(saber.left);
    if (style==Style::Gradient) return techni::gradient(options,now,saber.left);
    if (style==Style::WarmCold) return techni::palette(saber.left ? techni::warmPalette : techni::coldPalette,now+(saber.left ? options.leftOffset() : 0));
    if (now-saber.lastTime>=1 || now<saber.lastTime) {
        saber.from=saber.to;
        auto left=std::find_if(sabers.begin(),sabers.end(),[](const SaberModel& s){return s.left && s.model;});
        if(!saber.left && options.desync && options.saberStyle(true)==Style::PureRandom && left!=sabers.end()) saber.to=left->to;
        else saber.to=ChooseFlat(saber.left,Style::PureRandom);
        saber.lastTime=now;
    }
    return techni::lerp(saber.from,saber.to,now-saber.lastTime);
}
void ApplySaber(SaberModel& saber, techni::Color color) {
    if (!saber.model || !Visible(saber.model.ptr())) return;
    auto model=saber.model.ptr(); saberOverride=color; recoloringSaber=true;
    if (auto glows=model->_setSaberGlowColors) for (auto glow:glows) if(glow) glow->SetColors();
    if (auto glows=model->_setSaberFakeGlowColors) for (auto glow:glows) if(glow) glow->SetColors();
    if (model->_saberLight) model->_saberLight->set_color(ToUnity(color));
    if (model->_saberTrail) {
        auto tint=saber.trailTint;
        model->_saberTrail->_color=Color(color.r*tint.r,color.g*tint.g,color.b*tint.b,tint.a).get_linear();
    }
    recoloringSaber=false; saberOverride.reset();
}
bool HasMapEffects(GameplayCoreSceneSetupData* data) {
    if (!data || !data->beatmapLevel) return false;
    auto custom=il2cpp_utils::try_cast<SongCore::SongLoader::CustomBeatmapLevel>(data->beatmapLevel);
    if (!custom) return false;
    auto info=custom.value()->get_CustomSaveDataInfo();
    if (!info) return false;
    auto key=data->beatmapKey;
    if (!key.beatmapCharacteristic) return false;
    auto details=info->get().TryGetCharacteristicAndDifficulty(static_cast<std::string>(key.beatmapCharacteristic->get_serializedName()),key.difficulty);
    if (!details) return false;
    auto isEffect=[](const std::string& v) { return v=="Chroma" || v=="Noodle Extensions" || v=="NoodleExtensions"; };
    const auto& d=details->get();
    return std::any_of(d.requirements.begin(),d.requirements.end(),isEffect) || std::any_of(d.suggestions.begin(),d.suggestions.end(),isEffect);
}
LightState& Group(LightWithIdManager* manager,int id) {
    auto [it,inserted]=lightManagers.try_emplace(manager);
    if(inserted) it->second.manager=manager;
    return it->second.groups[id];
}
void LightEvent(LightWithIdManager* manager,int id,bool warm) {
    if (!Running() || !manager || id<0) return;
    auto& group=Group(manager,id); group.warm=warm;
    if(options.lights==Style::Off || options.lights==Style::Gradient) return;
    if(options.grouping==Grouping::Mayhem) { group.individual.clear(); return; }
    if(techniRandom.sample()>=options.frequency) return;
    auto hue=ChooseFlat(warm,options.lights);
    if(options.grouping==Grouping::Standard) globalLightHue=hue; else group.hue=hue;
}
Color GroupColor(const LightState& group) {
    auto source=ToNative(group.original);
    if(!options.enabled || options.lights==Style::Off) { source.a*=1+options.boost; return ToUnity(source); }
    if(options.lights==Style::Gradient) return ToUnity(techni::lightColor(source,techni::gradient(options,now,group.warm),options.boost));
    auto hue=options.grouping==Grouping::Standard ? globalLightHue : group.hue;
    if(hue) return ToUnity(techni::lightColor(source,*hue,options.boost));
    source.a*=1+options.boost; return ToUnity(source);
}
void ApplyParticles(ParticleEffect& effect) {
    if(!effect.effect || !effect.effect->_particleSystem || !effect.effect->_lightIsOn) return;
    auto self=effect.effect.ptr();
    auto hue=options.lights==Style::Gradient ? std::optional(techni::gradient(options,now,effect.warm)) : effect.hue;
    if(!hue) return;
    auto c=ToUnity(techni::lightColor(ToNative(self->_particleColor),*hue,options.boost));
    auto module=self->_mainModule;
    auto gradient=UnityEngine::ParticleSystem_MinMaxGradient(); gradient._ctor(c); module.set_startColor(gradient);
    if(options.lights!=Style::Gradient) return;
    if(!self->_particles) return;
    auto particles=self->_particles;
    int count=self->_particleSystem->GetParticles(byref(particles),particles.size());
    for(int i=0;i<count;++i) {
        auto color=*hue;
        if(options.lights==Style::Gradient && options.grouping==Grouping::Mayhem) color=techni::gradient(options,now,effect.warm,(i%1000)*.001f);
        // Flat Mayhem particles receive per-particle colors in the beatmap callback;
        // only smooth modes are rewritten each frame.
        auto p=ToUnity(techni::lightColor(ToNative(self->_particleColor),color,options.boost));
        particles[i].set_startColor(UnityEngine::Color32::op_Implicit___UnityEngine__Color32(p));
    }
    self->_particleSystem->SetParticles(byref(particles),count);
}
void Tick() {
    if(!Running()) return;
    now=UnityEngine::Time::get_time(); // PC uses Unity Time.time, including its pause behavior.
    std::erase_if(notes,[](const Note& n){return !n.visual;});
    std::erase_if(bombs,[](const Bomb& b){return !b.controller;});
    std::erase_if(walls,[](const Wall& w){return !w.controller;});
    std::erase_if(sabers,[](const SaberModel& s){return !s.model;});
    for(auto& n:notes) if(n.style==Style::Gradient && Visible(n.visual.ptr())) ColorNote(n.visual.ptr(),techni::gradient(options,now,n.left));
    for(auto& b:bombs) if(options.bombs==Style::Gradient && Visible(b.controller.ptr())) ColorBomb(b,techni::globalGradient(options,now));
    for(auto& w:walls) if(options.walls==Style::Gradient && Visible(w.controller.ptr())) ColorWall(w.controller.ptr(),techni::globalGradient(options,now),w.original.a);
    // Left first makes the PC "Desync" random-palette matching independent of prefab order.
    for(bool left:{true,false}) for(auto& s:sabers) if(s.left==left && options.saberStyle(s.left)!=Style::Off) ApplySaber(s,SaberColor(s));
    if(options.disableBackground || options.lights==Style::Off) return;
    for(auto& background:backgrounds) {
        if(!background || !Visible(background.ptr()) || !background->_bloomPrePassBackgroundColorsGradient) continue;
        auto gradient=background->_bloomPrePassBackgroundColorsGradient;
        auto src=background->_elements; auto dst=gradient->get_elements();
        if(!src || !dst) continue;
        for(int i=0;i<std::min(src.size(),dst.size());++i) {
            auto s=src[i]; auto d=dst[i]; if(!s || !d || !s->loadFromColorScheme) continue;
            int type=static_cast<int>(s->environmentColor);
            if(type<0 || type>3) continue;
            auto c=techni::gradient(options,now,(type%2)==0);
            d->color={c.r*s->intensity,c.g*s->intensity,c.b*s->intensity,s->intensity};
        }
        gradient->UpdateGradientTexture();
    }
}
}

MAKE_HOOK_MATCH(TechniInstall,&GameplayCoreInstaller::InstallBindings,void,GameplayCoreInstaller* self) {
    Reset(); options=ReadOptions();
    inLevel=true; now=UnityEngine::Time::get_time();
    if(HasMapEffects(self->_sceneSetupData)) { options.enabled=false; options.boost=0; options.disableBackground=false; PaperLogger.info("Map declares Chroma/Noodle: Techniqolor suspended for this level"); }
    TechniInstall(self);
    PaperLogger.info("Level prepared: enabled={}, light style={}, grouping={}",options.enabled,static_cast<int>(options.lights),static_cast<int>(options.grouping));
}
MAKE_HOOK_MATCH(TechniClockStart,&AudioTimeSyncController::Start,void,AudioTimeSyncController* self) {
    TechniClockStart(self); if(inLevel && !clockOwner) clockOwner=self;
}
MAKE_HOOK_MATCH(TechniClockUpdate,&AudioTimeSyncController::Update,void,AudioTimeSyncController* self) {
    TechniClockUpdate(self); if(self==clockOwner) Tick();
}
MAKE_HOOK_MATCH(TechniClockDestroy,&AudioTimeSyncController::OnDestroy,void,AudioTimeSyncController* self) {
    if(self==clockOwner) Reset(); TechniClockDestroy(self);
}
MAKE_HOOK_MATCH(TechniNote,&ColorNoteVisuals::HandleNoteControllerDidInit,void,ColorNoteVisuals* self,NoteControllerBase* controller) {
    TechniNote(self,controller);
    if(!Running() || !controller || !controller->get_noteData()) return;
    bool left=controller->get_noteData()->get_colorType()==ColorType::ColorA;
    auto style=options.noteStyle(left);
    std::erase_if(notes,[self](const Note& n){return !n.visual || n.visual.ptr()==self;});
    if(style==Style::Off) return;
    notes.push_back({self,left,style});
    ColorNote(self,style==Style::Gradient ? techni::gradient(options,now,left) : ChooseFlat(left,style));
}
MAKE_HOOK_MATCH(TechniBomb,&BombNoteController::Init,void,BombNoteController* self,NoteData* data,ByRef<NoteSpawnData> spawn) {
    TechniBomb(self,data,spawn);
    if(!Running() || options.bombs==Style::Off) return;
    std::erase_if(bombs,[self](const Bomb& b){return !b.controller || b.controller.ptr()==self;});
    Bomb b{self,{},Color(1,1,1,1)};
    auto blocks=self->GetComponentsInChildren<MaterialPropertyBlockController*>(true);
    for(auto block:blocks) if(block) b.blocks.emplace_back(block);
    if(!b.blocks.empty()) b.original=b.blocks.front()->get_materialPropertyBlock()->GetColor(colorId);
    // A never-initialized material property block has alpha zero; bombs must stay visible.
    if(b.original.a==0) b.original.a=1;
    ColorBomb(b,options.bombs==Style::Gradient ? techni::globalGradient(options,now) : ChooseFlat(true,options.bombs));
    bombs.push_back(std::move(b));
}
MAKE_HOOK_MATCH(TechniWall,&ObstacleController::Init,void,ObstacleController* self,ObstacleData* data,ByRef<ObstacleSpawnData> spawn) {
    TechniWall(self,data,spawn);
    if(!Running() || options.walls==Style::Off) return;
    std::erase_if(walls,[self](const Wall& w){return !w.controller || w.controller.ptr()==self;});
    auto original=self->get_color(); walls.push_back({self,original});
    ColorWall(self,options.walls==Style::Gradient ? techni::globalGradient(options,now) : ChooseFlat(true,options.walls),original.a);
}
MAKE_HOOK_MATCH(TechniSaber,&SaberModelController::Init,void,SaberModelController* self,UnityEngine::Transform* parent,Saber* saber,Color tint) {
    TechniSaber(self,parent,saber,tint);
    if(!Running() || !saber) return;
    bool left=saber->get_saberType()==SaberType::SaberA;
    if(options.saberStyle(left)==Style::Off && options.noteStyle(left)==Style::Off) return;
    std::erase_if(sabers,[self](const SaberModel& s){return !s.model || s.model.ptr()==self;});
    sabers.push_back({self,left,tint,ChooseFlat(left,Style::PureRandom),ChooseFlat(left,Style::PureRandom),now});
}
MAKE_HOOK_MATCH(TechniSaberColor,&ColorManager::ColorForSaberType,Color,ColorManager* self,SaberType type) {
    if(recoloringSaber && saberOverride) return ToUnity(*saberOverride);
    return TechniSaberColor(self,type);
}
MAKE_HOOK_MATCH(TechniSaberEffectsColor,&ColorManager::EffectsColorForSaberType,Color,ColorManager* self,SaberType type) {
    if(recoloringSaber && saberOverride) return ToUnity(*saberOverride);
    return TechniSaberEffectsColor(self,type);
}
MAKE_HOOK_MATCH(TechniArc,&SliderController::UpdateMaterialPropertyBlock,void,SliderController* self,float timeSinceJump) {
    if(Running() && self->_sliderData) {
        bool left=self->_sliderData->get_colorType()==ColorType::ColorA;
        if(options.noteStyle(left)==Style::Gradient) self->_initColor=ToUnity(techni::gradient(options,now,left));
    }
    TechniArc(self,timeSinceJump);
}
MAKE_HOOK_MATCH(TechniBackground,&BloomPrePassBackgroundColorsGradientFromColorSchemeColors::Start,void,BloomPrePassBackgroundColorsGradientFromColorSchemeColors* self) {
    TechniBackground(self);
    if(!Running()) return;
    if(options.disableBackground) self->get_gameObject()->SetActive(false);
    else backgrounds.emplace_back(self);
}
MAKE_HOOK_MATCH(TechniBasicLightEvent,&LightSwitchEventEffect::HandleColorChangeBeatmapEvent,void,LightSwitchEventEffect* self,BasicBeatmapEventData* event) {
    if(event) LightEvent(self->_lightManager.ptr(),self->_lightsID,event->value>3);
    TechniBasicLightEvent(self,event);
}
MAKE_HOOK_MATCH(TechniV3LightEvent,&LightColorGroupEffect::HandleColorChangeBeatmapEvent,void,LightColorGroupEffect* self,LightColorBeatmapEventData* event) {
    if(event) LightEvent(self->_lightManager.ptr(),self->_lightId,event->get_colorType()==EnvironmentColorType::Color1);
    TechniV3LightEvent(self,event);
}
MAKE_HOOK_MATCH(TechniSetLight,&LightWithIdManager::SetColorForId,void,LightWithIdManager* self,int id,Color color) {
    if(inLevel && id>=0 && (options.enabled || options.boost!=0)) {
        auto& group=Group(self,id); group.original=color;
        if(options.grouping!=Grouping::Mayhem || options.lights==Style::Off || !options.enabled) color=GroupColor(group);
    }
    TechniSetLight(self,id,color);
}
MAKE_HOOK_MATCH(TechniLightsLateUpdate,&LightWithIdManager::LateUpdate,void,LightWithIdManager* self) {
    auto it=lightManagers.find(self);
    if(!inLevel || it==lightManagers.end() || !it->second.manager) { TechniLightsLateUpdate(self); return; }
    if(options.grouping!=Grouping::Mayhem || options.lights==Style::Off || !options.enabled) {
        for(auto& [id,group]:it->second.groups) Hook_TechniSetLight::TechniSetLight(self,id,GroupColor(group));
        TechniLightsLateUpdate(self); return;
    }
    TechniLightsLateUpdate(self);
    auto lists=self->_lights;
    if(!lists) return;
    for(auto& [id,group]:it->second.groups) {
        if(id<0 || id>=lists.size() || !lists[id]) continue;
        auto list=lists[id];
        for(int i=0;i<list->get_Count();++i) {
            auto light=list->get_Item(i); if(!light || !light->get_isRegistered()) continue;
            techni::Color hue;
            if(options.lights==Style::Gradient) {
                float seed=(reinterpret_cast<uintptr_t>(light)%1000)*.001f;
                hue=techni::gradient(options,now,group.warm,seed);
            } else {
                auto p=group.individual.find(light);
                if(p==group.individual.end()) p=group.individual.emplace(light,ChooseFlat(group.warm,options.lights)).first;
                hue=p->second;
            }
            light->ColorWasSet(ToUnity(techni::lightColor(ToNative(group.original),hue,options.boost)));
        }
    }
}
MAKE_HOOK_MATCH(TechniCut,&NoteController::SendNoteWasCutEvent,void,NoteController* self,ByRef<NoteCutInfo> info) {
    bool left=info->saberType==SaberType::SaberA;
    std::optional<techni::Color> color;
    if(Running() && options.saberStyle(left)==Style::Off && info->noteData && static_cast<int>(info->noteData->get_colorType())==static_cast<int>(info->saberType)) {
        for(auto& note:notes) if(note.visual && note.visual->_noteController.ptr()==self) { color=ToNative(note.visual->_noteColor); break; }
    }
    TechniCut(self,info);
    if(color && Running()) for(auto& saber:sabers) if(saber.left==left) ApplySaber(saber,*color);
}
MAKE_HOOK_MATCH(TechniParticles,&ParticleSystemEventEffect::HandleBeatmapEvent,void,ParticleSystemEventEffect* self,BasicBeatmapEventData* event) {
    TechniParticles(self,event);
    if(!Running() || !event || options.lights==Style::Off || !self->_particleSystem || !self->_lightIsOn) return;
    auto& effect=particleEffects[self]; effect.effect=self; effect.warm=event->value>3;
    auto group=Grouping::Mayhem==options.grouping;
    if(options.lights!=Style::Gradient && techniRandom.sample()>=options.frequency) { effect.hue.reset(); return; }
    auto hue=options.lights==Style::Gradient ? techni::gradient(options,now,event->value>3) : ChooseFlat(event->value>3,options.lights);
    effect.hue=hue;
    auto c=ToUnity(techni::lightColor(ToNative(self->_particleColor),hue,options.boost));
    auto module=self->_mainModule;
    auto gradient=UnityEngine::ParticleSystem_MinMaxGradient(); gradient._ctor(c); module.set_startColor(gradient);
    if(!group || !self->_particles) return;
    auto particles=self->_particles;
    int count=self->_particleSystem->GetParticles(byref(particles),particles.size());
    for(int i=0;i<count;++i) {
        auto color=options.lights==Style::Gradient ? techni::gradient(options,now,event->value>3,(i%1000)*.001f) : ChooseFlat(event->value>3,options.lights);
        auto p=ToUnity(techni::lightColor(ToNative(self->_particleColor),color,options.boost));
        particles[i].set_startColor(UnityEngine::Color32::op_Implicit___UnityEngine__Color32(p));
    }
    self->_particleSystem->SetParticles(byref(particles),count);
}
MAKE_HOOK_MATCH(TechniParticlesUpdate,&ParticleSystemEventEffect::Update,void,ParticleSystemEventEffect* self) {
    TechniParticlesUpdate(self);
    if(!Running() || options.lights==Style::Off) return;
    auto it=particleEffects.find(self); if(it!=particleEffects.end()) ApplyParticles(it->second);
}
extern "C" __attribute__((visibility("default"))) void setup(CModInfo* info) noexcept {
    *info=modInfo.to_c(); Paper::Logger::RegisterFileContextId(PaperLogger.tag);
}
extern "C" __attribute__((visibility("default"))) void late_load() noexcept {
    il2cpp_functions::Init(); getTechniqolorConfig().Init(modInfo);
    auto gameVersion=static_cast<std::string>(UnityEngine::Application::get_version());
    if(gameVersion!="1.40.8" && gameVersion!="1.40.8_7379") {
        PaperLogger.error("Unsupported game version {}; expected 1.40.8_7379. Hooks were not installed.",gameVersion); return;
    }
    colorId=UnityEngine::Shader::PropertyToID("_Color"); simpleColorId=UnityEngine::Shader::PropertyToID("_SimpleColor");
    RegisterTechniqolorSettings();
    INSTALL_HOOK(PaperLogger,TechniInstall);
    INSTALL_HOOK(PaperLogger,TechniClockStart);
    INSTALL_HOOK(PaperLogger,TechniClockUpdate);
    INSTALL_HOOK(PaperLogger,TechniClockDestroy);
    INSTALL_HOOK(PaperLogger,TechniNote);
    INSTALL_HOOK(PaperLogger,TechniBomb);
    INSTALL_HOOK(PaperLogger,TechniWall);
    INSTALL_HOOK(PaperLogger,TechniSaber);
    INSTALL_HOOK(PaperLogger,TechniSaberColor);
    INSTALL_HOOK(PaperLogger,TechniSaberEffectsColor);
    INSTALL_HOOK(PaperLogger,TechniArc);
    INSTALL_HOOK(PaperLogger,TechniBackground);
    INSTALL_HOOK(PaperLogger,TechniBasicLightEvent);
    INSTALL_HOOK(PaperLogger,TechniV3LightEvent);
    INSTALL_HOOK(PaperLogger,TechniSetLight);
    INSTALL_HOOK(PaperLogger,TechniLightsLateUpdate);
    INSTALL_HOOK(PaperLogger,TechniCut);
    INSTALL_HOOK(PaperLogger,TechniParticles);
    INSTALL_HOOK(PaperLogger,TechniParticlesUpdate);
    PaperLogger.info("Techniqolor {} loaded, target Beat Saber 1.40.8_7379",VERSION);
}
