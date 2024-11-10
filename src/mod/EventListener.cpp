#include "Global.h"
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/schedule/Scheduler.h>
#include <ll/api/schedule/Task.h>
#include <ll/api/utils/StringUtils.h>
#include <ll/api/Config.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/event/command/ExecuteCommandEvent.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerLeaveEvent.h>
#include <ll/api/service/Bedrock.h>
#include <mc/deps/core/mce/UUID.h>
#include <mc/deps/core/utility/BinaryStream.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/packet/AddActorBasePacket.h>
#include <mc/network/packet/BossEventPacket.h>
#include <mc/network/packet/Packet.h>
#include <mc/network/packet/RemoveActorPacket.h>
#include <mc/network/packet/TextPacket.h>
#include <mc/server/LoopbackPacketSender.h>
#include <mc/world/ActorUniqueID.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/events/BossEventUpdateType.h>
#include <mc/world/level/Level.h>
#include <mc/deps/application/AppPlatform.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/commands/CommandContext.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/world/actor/player/Player.h>
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
void saveResults(std::vector<std::string> results, mce::UUID& uuid) {
    auto level = ll::service::getLevel();
    if (!level.has_value()) return;
    auto* player                             = level->getPlayer(uuid);
    auto& playerConfig                       = data.playerConfigs;
    playerConfig[uuid.asString()].location  = results;
    playerConfig[uuid.asString()].realName = player->getRealName();
    playerConfig[uuid.asString()].deviceName = getPlayerDeviceName(uuid);
    playerConfig[uuid.asString()].isCached   = true;
    setPlayerConfig(uuid, playerConfig[uuid.asString()]);
}
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
            
            logger.warn(SplitIpPort(ipAndPort).first);
            getLocation(SplitIpPort(ipAndPort).first, saveResults,uuid);
            // setPlayerConfig(uuid,)
        });
    } else {
        eventBus.removeListener(joinEventListener);
    }
}