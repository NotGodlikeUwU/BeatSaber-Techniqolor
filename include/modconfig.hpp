#pragma once
#include "config-utils/shared/config-utils.hpp"
#include "color_engine.hpp"
DECLARE_CONFIG(TechniqolorConfig) {
    CONFIG_VALUE(TechnicolorEnabled, bool, "Enable Techniqolor", true);
    CONFIG_VALUE(TechnicolorLightsStyle, int, "Lights style: 0 Off, 1 Warm/Cold, 2 Random, 3 Gradient", 3);
    CONFIG_VALUE(TechnicolorLightsGrouping, int, "Grouping: 0 Standard, 1 Isolated Event, 2 Mayhem", 2);
    CONFIG_VALUE(TechnicolorLightsFrequency, float, "Lights frequency", .1f);
    CONFIG_VALUE(TechnicolorSabersStyle, int, "Sabers style", 0);
    CONFIG_VALUE(TechnicolorBlocksStyle, int, "Notes style", 0);
    CONFIG_VALUE(TechnicolorWallsStyle, int, "Walls style", 3);
    CONFIG_VALUE(TechnicolorBombsStyle, int, "Bombs style", 2);
    CONFIG_VALUE(Desync, bool, "Desync left/right (PC behavior)", false);
    CONFIG_VALUE(DisableGradientBackground, bool, "Disable gradient background", false);
    CONFIG_VALUE(ColorBoost, float, "Color boost", 0.f);
    CONFIG_VALUE(UseLeftSaberStyle, bool, "Use separate left saber style", false);
    CONFIG_VALUE(LeftTechnicolorSabersStyle, int, "Left saber style", 0);
    CONFIG_VALUE(UseLeftBlocksStyle, bool, "Use separate left notes style", false);
    CONFIG_VALUE(LeftTechnicolorBlocksStyle, int, "Left notes style", 0);
};
inline techni::Options ReadOptions() {
    auto& c=getTechniqolorConfig(); techni::Options o;
    o.enabled=c.TechnicolorEnabled.GetValue(); o.desync=c.Desync.GetValue();
    o.disableBackground=c.DisableGradientBackground.GetValue();
    o.useLeftNotes=c.UseLeftBlocksStyle.GetValue(); o.useLeftSaber=c.UseLeftSaberStyle.GetValue();
    o.lights=techni::validStyle(c.TechnicolorLightsStyle.GetValue());
    o.notes=techni::validStyle(c.TechnicolorBlocksStyle.GetValue());
    o.walls=techni::validStyle(c.TechnicolorWallsStyle.GetValue());
    o.bombs=techni::validStyle(c.TechnicolorBombsStyle.GetValue());
    o.sabers=techni::validStyle(c.TechnicolorSabersStyle.GetValue());
    o.leftNotes=techni::validStyle(c.LeftTechnicolorBlocksStyle.GetValue());
    o.leftSaber=techni::validStyle(c.LeftTechnicolorSabersStyle.GetValue());
    o.grouping=static_cast<techni::Grouping>(std::clamp(c.TechnicolorLightsGrouping.GetValue(),0,2));
    o.frequency=techni::finiteClamp(c.TechnicolorLightsFrequency.GetValue(),0,1,.1f);
    o.boost=techni::finiteClamp(c.ColorBoost.GetValue(),-1,2,0);
    return o;
}
