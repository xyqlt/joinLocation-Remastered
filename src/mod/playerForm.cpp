#pragma once
#include "Config.h"
#include "Global.h"
#include "fmt/core.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/mce/UUID.h"
#include <ll/api/form/CustomForm.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/level/Level.h>

bool sendPlayerForm(mce::UUID& uuid) {
    auto level = ll::service::getLevel();
    if (!level.has_value()) return false;
    auto* player = level->getPlayer(uuid);
    if (!player) return false;
    auto form = ll::form::CustomForm();
    if (player->isOperator()) {
        std::vector<std::string> playerMaps;
        level->forEachPlayer([&](Player& player) -> bool {
            playerMaps.push_back(player.getRealName());
            return true;
        });
        form.setTitle("§e查询信息");
        form.appendLabel("当前版本：" + std::string(VERSION.to_string()));
        form.appendLabel("当前配置版本：" + std::to_string(config.version));
        form.appendLabel(fmt::format("当前配置是否启用缓存：{}", config.enableCache ? "是" : "否"));
        form.appendDropdown("playerDropDown", "选择玩家", playerMaps);
        form.appendInput("url", "输入IP地址或域名");
        form.appendToggle("enableCache", "是否以缓存查询");
        form.appendToggle("enableQueryUrl", "是否以url查询");
        form.sendTo(
            *player,
            [](Player& player, ll::form::CustomFormResult const& result, ll::form::FormCancelReason reason) -> void {
                if (!result.has_value()) {
                    player.sendMessage("§c表单已放弃");
                    return;
                }
            }
        );


    } else {
    }
    return true;
}
bool sendConfigForm(mce::UUID& uuid) {
    auto level = ll::service::getLevel();
    if (!level.has_value()) return false;
    auto* player = level->getPlayer(uuid);
    if (!player) return false;
    auto form = ll::form::CustomForm();
    form.setTitle("§e设置");
    form.appendLabel("§e当前配置文件版本: " + std::to_string(config.version));
    form.appendToggle("enabledPAPI", "是否启用PAPI变量注册", config.enabledPAPI);
    form.appendToggle("enabledChat", "是否启用全局聊天栏显示", config.enabledChat);
    form.appendToggle("enabledToast", "是否启用全局通知", config.enabledToast);
    form.appendToggle("enableCache", "是否启用缓存", config.enableCache);
    form.appendToggle("enableRegisterCommand", "是否启用注册命令", config.enableRegisterCommand);
    form.appendInput("command", "自定义命令", "自定义命令", config.command);
    form.appendInput("alias", "自定义别名", "自定义别名", config.alias);
    form.sendTo(
        *player,
        [](Player& player, ll::form::CustomFormResult const& result, ll::form::FormCancelReason reason) -> void {
            if (!result.has_value()) {
                player.sendMessage("§c表单已放弃");
                return;
            }
            setGlobalConfig(join_location::Config{
                config.version,
                static_cast<bool>(std::get<uint64>(result->at("enabledPAPI"))),
                static_cast<bool>(std::get<uint64>(result->at("enabledChat"))),
                static_cast<bool>(std::get<uint64>(result->at("enabledToast"))),
                static_cast<bool>(std::get<uint64>(result->at("enableCache"))),
                static_cast<bool>(std::get<uint64>(result->at("enableRegisterCommand"))),
                std::get<std::string>(result->at("command")),
                std::get<std::string>(result->at("alias"))
            });
        }
    );
    return true;
}