#include "Global.h"
#include "mc/server/commands/CommandPermissionLevel.h"

#include <ll/api/chrono/GameChrono.h>
#include <ll/api/schedule/Scheduler.h>
#include <ll/api/schedule/Task.h>
#include <ll/api/utils/StringUtils.h>
#include <variant>


#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/event/command/ExecuteCommandEvent.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerLeaveEvent.h>


#include <mc/deps/application/AppPlatform.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/commands/CommandContext.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/world/actor/player/Player.h>
#include <vector>


ll::event::ListenerPtr joinEventListener;
void                   ListenerCall(bool enable) {
    auto& eventBus = ll::event::EventBus::getInstance();
    if (enable) {

        joinEventListener = eventBus.emplaceListener<ll::event::PlayerJoinEvent>([](ll::event::PlayerJoinEvent& event) {
            if (event.self().isSimulatedPlayer()) return;
            auto& uuid       = event.self().getUuid();
            auto  name       = event.self().getRealName();
            auto  ipAndPort  = event.self().getIPAndPort();
            auto  deviceName = std::string(magic_enum::enum_name(event.self().getPlatform()));
            // event.self().sendMessage(uuid.asString());
            // event.self().sendMessage(name);
            // event.self().sendMessage(ip);
            // event.self().sendMessage(deviceName);
            auto saveResults = [&](std::vector<std::string> results) {
                auto& playerConfig    = config.playerConfigs[event.self().getUuid()];
                playerConfig.location = results;
                playerConfig.realName = event.self().getRealName();
                playerConfig.deviceName = std::string(magic_enum::enum_name(event.self().getPlatform()));
                playerConfig.isCached = true;
                auto uuid             = event.self().getUuid();
                setPlayerConfig(uuid, playerConfig);
            };
            logger.warn(SplitIpPort(ipAndPort).first);
            getLocation(SplitIpPort(ipAndPort).first, saveResults);
            // setPlayerConfig(uuid,)
        });
    } else {
        eventBus.removeListener(joinEventListener);
    }
}