#pragma once
#include "Global.h"
#include "global.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/mce/UUID.h"
#include <ll/api/Config.h>
#include <ll/api/command/Command.h>
#include <ll/api/command/CommandHandle.h>
#include <ll/api/command/CommandRegistrar.h>
#include <mc/server/commands/CommandOrigin.h>
#include <mc/server/commands/CommandOutput.h>
#include <mc/server/commands/CommandPermissionLevel.h>
#include <mc/server/commands/CommandStatus.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>
namespace Action {
enum Type { url, name };
struct Commands {
    Type        type;
    std::string url;
    std::string name;
};
}; // namespace Action
void registerCommands() {
    auto commandRegistry = ll::service::getCommandRegistry();
    if (!commandRegistry) {
        throw std::runtime_error("failed to get command registry");
    }

    auto& command = ll::command::CommandRegistrar::getInstance()
                        .getOrCreateCommand(config.command, "查询ip地址", CommandPermissionLevel::Any);
    if (!config.alias.empty()) command.alias(config.alias);
    command.overload<Action::Commands>().text("config").text("gui").execute(
        [](CommandOrigin const& ori, CommandOutput& output, Action::Commands const& param) {
            auto* entity = ori.getEntity();
            if (entity == nullptr || !entity->isPlayer()) {
                return output.error("控制台不支持此命令");
            }
            auto* player = static_cast<Player*>(entity);
            if (!player->isOperator()) {
                return output.error("你没有权限使用此命令");
            }
            auto uuid = player->getUuid();

            sendConfigForm(uuid);
        }
    );
    command.overload<Action::Commands>().text("query").text("gui").execute(
        [](CommandOrigin const& ori, CommandOutput& output, Action::Commands const& param) {
            auto* entity = ori.getEntity();
            if (entity == nullptr || !entity->isPlayer()) {
                return output.error("控制台不支持此命令");
            }
            auto* player = static_cast<Player*>(entity);
            auto  uuid   = player->getUuid();
            sendPlayerForm(uuid);
        }
    );
}