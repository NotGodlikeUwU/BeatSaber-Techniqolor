#pragma once
#include "scotland2/shared/modloader.h"
#include "beatsaber-hook/shared/utils/hooking.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "paper2_scotland2/shared/logger.hpp"
constexpr auto PaperLogger = Paper::ConstLoggerContext("techniqolor");
void RegisterTechniqolorSettings();
