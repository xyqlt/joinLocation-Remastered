#pragma once

#include "Config.h"
#include "mc/deps/core/mce/UUID.h"

#include <ll/api/Logger.h>
#include <ll/api/data/Version.h>

#define VERSION ll::data::Version(2, 0, 0)

extern ll::Logger            logger;
extern join_location::Config config;
extern join_location::Data   Gdata;
extern void                  ListenerCall(bool enable);
extern void                  getLocation(
                     std::string                                                           ip,
                     const std::function<void(std::vector<std::string>, mce::UUID& uuid)>& callback,
                     const mce::UUID&                                                      uuidA
                 );
extern std::pair<std::string, std::string> SplitIpPort(const std::string& ipAndPort);
extern std::string                         getPlayerDeviceName(mce::UUID& uuid);
extern bool                                displayNotice(mce::UUID& uuid);
extern void                                registerCommands();
extern void                                setPlayerConfig(mce::UUID& uuid, join_location::PlayerConfig& playerConfig);
extern bool                                sendConfigForm(mce::UUID& uuid);
extern bool                                sendPlayerForm(mce::UUID& uuid);
extern void                                setGlobalConfig(join_location::Config config);
extern std::string                         resolvesDomain(const std::string& domain);
extern bool                                isIP(const std::string& ip);
// extern void PapiCall(bool enable);