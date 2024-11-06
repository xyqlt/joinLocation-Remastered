#pragma once

#include "Config.h"

#include <ll/api/Logger.h>
#include <ll/api/data/Version.h>

#define VERSION ll::data::Version(2, 0, 4)

extern ll::Logger     logger;
extern join_location::Config config;
//extern void registerCommands();
//extern void setPlayerConfig(mce::UUID& uuid, join_location::PlayerConfig playerConfig);
///extern void PapiCall(bool enable);