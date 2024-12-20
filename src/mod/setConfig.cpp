#include "Entry.h"
#include "Global.h"
#include <ll/api/Config.h>
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

void setPlayerConfig(mce::UUID& uuid, join_location::PlayerConfig& playerConfig) {
    auto oldPlayerConfig = Gdata.playerConfigs[uuid];
    auto level           = ll::service::getLevel();
    if (!level.has_value()) return;
    Gdata.playerConfigs[uuid] = playerConfig;
    ll::config::saveConfig(Gdata, join_location::Entry::getInstance().getSelf().getConfigDir() / "data.json");
}
void setGlobalConfig(join_location::Config newConfig) {
    config.version               = newConfig.version;
    config.enableCache           = newConfig.enableCache;
    config.enableRegisterCommand = newConfig.enableRegisterCommand;
    config.enabledChat           = newConfig.enabledChat;
    config.enabledPAPI           = newConfig.enabledPAPI;
    config.enabledToast          = newConfig.enabledToast;
    config.alias                 = newConfig.alias;
    config.command               = newConfig.command;
    ll::config::saveConfig(config, join_location::Entry::getInstance().getSelf().getConfigDir() / "config.json");
}