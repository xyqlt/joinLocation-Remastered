#include "Config.h"
#include "Global.h"
#include <ll/api/Config.h>
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/ListenerBase.h>
#include <ll/api/event/command/ExecuteCommandEvent.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerLeaveEvent.h>
#include <ll/api/schedule/Scheduler.h>
#include <ll/api/schedule/Task.h>
#include <ll/api/service/Bedrock.h>
#include <ll/api/utils/StringUtils.h>
#include <mc/deps/application/AppPlatform.h>
#include <mc/deps/core/mce/UUID.h>
#include <mc/deps/core/utility/BinaryStream.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/network/NetworkConnection.h>
#include <mc/network/packet/AddActorBasePacket.h>
#include <mc/network/packet/BossEventPacket.h>
#include <mc/network/packet/Packet.h>
#include <mc/network/packet/RemoveActorPacket.h>
#include <mc/network/packet/TextPacket.h>
#include <mc/server/LoopbackPacketSender.h>
#include <mc/server/commands/CommandContext.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/world/ActorUniqueID.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/events/BossEventUpdateType.h>
#include <mc/world/level/Level.h>


#include <mc/deps/application/AppPlatform.h>
#include <mc/entity/utilities/ActorType.h>
#include <mc/server/commands/CommandContext.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/world/actor/player/Player.h>
#include <vector>


ll::event::ListenerPtr joinEventListener;
void                   saveResults(std::vector<std::string> results, mce::UUID& uuid) {
    auto level = ll::service::getLevel();
    if (!level.has_value()) return;
    auto* player                             = level->getPlayer(uuid);
    auto& playerConfig                       = playerData.playerConfigs;
    playerConfig[uuid.asString()].location   = results;
    playerConfig[uuid.asString()].realName   = player->getRealName();
    playerConfig[uuid.asString()].deviceName = getPlayerDeviceName(uuid);
    playerConfig[uuid.asString()].ip         = SplitIpPort(player->getIPAndPort()).first;
    playerConfig[uuid.asString()].isCached   = true;
    setPlayerConfig(uuid, playerConfig[uuid.asString()]);
}
void ListenerCall(bool enable) {
    auto& eventBus = ll::event::EventBus::getInstance();
    if (enable) {
        joinEventListener = eventBus.emplaceListener<ll::event::PlayerJoinEvent>([](ll::event::PlayerJoinEvent& event) {
            if (event.self().isSimulatedPlayer()) return;
            auto uuid       = event.self().getUuid();
            auto name       = event.self().getRealName();
            auto ipAndPort  = event.self().getIPAndPort();
            auto deviceName = getPlayerDeviceName(uuid);
            if (config.enableCache) {
                if (playerData.playerConfigs[uuid].isCached) {
                    logger.warn("玩家{}信息没有缓存,正在更新...", name);
                    getLocation(playerData.playerConfigs[uuid].ip, saveResults, uuid);
                    displayNotice(uuid);
                } else if (playerData.playerConfigs[uuid].ip != SplitIpPort(ipAndPort).first
                           || playerData.playerConfigs[uuid].deviceName != deviceName
                           || playerData.playerConfigs[uuid].realName != name) {
                    logger.warn("玩家{}信息发生变化正在更新...", name);
                    getLocation(SplitIpPort(ipAndPort).first, saveResults, uuid);
                    displayNotice(uuid);
                } else {
                    displayNotice(uuid);
                }
            }else{

            }
        });
    } else {
        eventBus.removeListener(joinEventListener);
    }
}