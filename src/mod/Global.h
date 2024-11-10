#pragma once

#include "Config.h"

#include <ll/api/Logger.h>
#include <ll/api/data/Version.h>

#define VERSION ll::data::Version(2, 0, 0)

extern ll::Logger     logger;
extern join_location::Config config;
extern void ListenerCall(bool enable);
extern void getLocation(std::string ip, const std::function<void(std::vector<std::string>)>& callback);
extern std::pair<std::string,std::string> SplitIpPort(const std::string& ipAndPort);
//extern void registerCommands();
extern void setPlayerConfig(mce::UUID& uuid, join_location::PlayerConfig& playerConfig);
///extern void PapiCall(bool enable);