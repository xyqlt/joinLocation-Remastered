#pragma once
#include "Global.h"
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/schedule/Scheduler.h>
#include <ll/api/schedule/Task.h>
#include <ll/api/utils/StringUtils.h>
#include <string>
#include <unordered_map>


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

std::string getPlayerDeviceName(mce::UUID& uuid) {
    auto level = ll::service::getLevel();
    if (!level.has_value()) return "";
    auto* player = level->getPlayer(uuid);
    if (!player) return "";
    auto                                         deviceName = std::string(magic_enum::enum_name(player->getPlatform()));
    std::unordered_map<std::string, std::string> device{
        {"Android",     "安卓设备"     },
        {"iOS",         "苹果设备"     },
        {"OSX",         "macOS设备"      },
        {"Amazon",      "亚马逊设备"  },
        {"GearVR",      "三星GearVR"     },
        {"Hololens",    "微软HoloLens"   },
        {"Windows10",   "Windows"          },
        {"Win32",       "Windows"          },
        {"UWP",         "Windows"          },
        {"windows",     "Windows"          },
        {"PlayStation", "PlayStation主机"},
        {"Nintendo",    "任天堂Switch"  },
        {"Xbox",        "Xbox主机"       },
        {"TVOS",        "苹果tvOS"       }
    };
    if(device.count(deviceName)==0){
        return "未知设备";
    }else {
        return device[deviceName];
    }
}




std::pair<std::string, std::string> SplitIpPort(const std::string& ip_port) {
    std::string ip, port;
    size_t      pos = ip_port.find(':');
    if (pos != std::string::npos) {
        ip   = ip_port.substr(0, pos);
        port = ip_port.substr(pos + 1);
    } else {
        // 如果没有找到冒号，可以处理错误情况
        ip   = ip_port;
        port = "";
    }
    return {ip, port};
}