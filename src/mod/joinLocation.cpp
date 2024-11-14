#include "Global.h"
#include "fmt/core.h"
#include "ll/api/service/Bedrock.h"
#include "mc/deps/core/mce/UUID.h"
#include "mc/network/ServerNetworkHandler.h"
#include "mc/network/packet/ToastRequestPacket.h"
#include "nlohmann/json_fwd.hpp"
#include <httplib.h>
#include <mc/entity/EntityContext.h>
#include <mc/entity/utilities/ActorEquipment.h>
#include <mc/entity/utilities/ActorMobilityUtils.h>
#include <mc/nbt/CompoundTag.h>
#include <mc/world/SimpleContainer.h>
#include <mc/world/actor/Actor.h>
#include <mc/world/actor/SynchedActorData.h>
#include <mc/world/actor/SynchedActorDataEntityWrapper.h>
#include <mc/world/actor/components/SynchedActorDataAccess.h>
#include <mc/world/actor/player/Player.h>
#include <mc/world/attribute/Attribute.h>
#include <mc/world/attribute/AttributeInstance.h>
#include <mc/world/attribute/SharedAttributes.h>
#include <mc/world/level/BlockSource.h>
#include <mc/world/level/Command.h>
#include <mc/world/level/IConstBlockSource.h>
#include <mc/world/level/Level.h>
#include <mc/world/level/biome/Biome.h>
#include <mc/world/level/material/Material.h>
#include <mc/world/scores/Objective.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>


using namespace std;

void SplitHttpUrl(const std::string& url, string& host, string& path) {
    std::string_view sv(url);

    size_t protocolPos = sv.find("://");
    if (protocolPos != std::string_view::npos) {
        sv = sv.substr(protocolPos + 3);
    }

    size_t splitPos = sv.find('/');
    if (splitPos == std::string_view::npos) {
        path = "/";
        host = std::string(sv);
    } else {
        path = std::string(sv.substr(splitPos));
        host = std::string(sv.substr(0, splitPos));
    }
}

bool HttpGet(
    const std::string&                           url,
    const httplib::Headers&                      headers,
    const std::function<void(int, std::string)>& callback,
    int                                          timeout = -1
) {
    string host, path;
    SplitHttpUrl(url, host, path);

    auto cli = std::make_unique<httplib::Client>(host.c_str());
    if (!cli->is_valid()) {
        return false;
    }
    if (timeout > 0) cli->set_connection_timeout(timeout, 0);

    std::thread([cli = std::move(cli), headers, callback, path{std::move(path)}]() {
        try {
            auto response = cli->Get(path.c_str(), headers);

            if (!response) {
                // logger.error("Error in HttpGet: " + std::string(response->reason));
                callback(-1, "");
            } else {
                // if (response->status == 200) {
                //     logger.info("HttpGet: " + std::string(response->reason));
                // } else {
                //     logger.error("HttpGet: " + std::string(response->reason));
                // }

                callback(response->status, response->body);
            }
        } catch (const std::exception& e) {
            logger.error("Error in HttpGet: " + std::string(e.what()));
            callback(-1, "");
        }
    }).detach();

    return true;
}

bool HttpGet(const std::string& url, const std::function<void(int, std::string)>& callback, int timeout = -1) {
    return HttpGet(url, {}, callback, timeout);
}
void safeCallback(
    const std::function<void(std::vector<std::string>, mce::UUID&)>& callback,
    const std::vector<std::string>&                                  result,
    const mce::UUID&                                                 uuidA
) {
    static std::mutex           mutex;
    std::lock_guard<std::mutex> lock(mutex);

    if (callback) {
        try {
            mce::UUID uuidCopy = uuidA; // 创建副本
            callback(result, uuidCopy);
        } catch (const std::exception& e) {
            logger.error("Exception in callback: {}", e.what());
        } catch (...) {
            logger.error("Unknown exception in callback!");
        }
    } else {
        logger.error("Callback is nullptr! UUID: {}", uuidA.asString());
        return;
    }
}
void getLocation(
    std::string                                                           ip,
    const std::function<void(std::vector<std::string>, mce::UUID& uuid)>& callback,
    const mce::UUID&                                                      uuidA
) {
    string url = "https://mesh.if.iqiyi.com/aid/ip/info?version=1.1.1&ip=" + ip;
    HttpGet(
        url,
        [callback, uuidA](int code, std::string body) {
            std::vector<std::string> result;
            // logger.warn("callback called");
            // logger.info("Body:" + body);
            // logger.info("Status:" + std::to_string(code));
            if (code == 200) {
                nlohmann::json dataJson = nlohmann::json::parse(body);
                if (dataJson["code"].get<std::string>() == "0") {
                    auto data = dataJson["data"];
                    logger.info("data:" + data.dump(4));
                    try {
                        if (data.contains("countryCN") && data["countryCN"].is_string()) {
                            logger.info("countryCN:" + data["countryCN"].get<std::string>());

                            result.push_back(
                                data["countryCN"].get<std::string>() == "*" ? "\0"
                                                                            : data["countryCN"].get<std::string>() + " "
                            );
                        }
                        if (data.contains("provinceCN") && data["provinceCN"].is_string()) {
                            logger.info("provinceCN:" + data["provinceCN"].get<std::string>());
                            result.push_back(
                                data["provinceCN"].get<std::string>() == "*"
                                    ? "\0"
                                    : data["provinceCN"].get<std::string>() + " "
                            );
                        }
                        if (data.contains("cityCN") && data["cityCN"].is_string()) {
                            logger.info("cityCN:" + data["cityCN"].get<std::string>());
                            result.push_back(
                                data["cityCN"].get<std::string>() == "*" ? "\0"
                                                                         : data["cityCN"].get<std::string>() + " "
                            );
                        }

                        if (data.contains("countyCN") && data["countyCN"].is_string()) {
                            std::string countyCN = data["countyCN"].get<std::string>();
                            logger.info("countyCN:" + countyCN);
                            if (countyCN != "*") {
                                result.push_back(countyCN == "*" ? "\0" : countyCN + " ");
                            } else {
                                if (data.contains("townCN") && data["townCN"].is_string()) {
                                    logger.info("townCN:" + data["townCN"].get<std::string>());
                                    result.push_back(
                                        data["townCN"].get<std::string>() == "*"
                                            ? "\0"
                                            : data["townCN"].get<std::string>() + " "
                                    );
                                } else {
                                    result.push_back("\0");
                                }
                            }
                        }

                        if (data.contains("ispCN") && data["ispCN"].is_string()) {
                            logger.info("ispCN:" + data["ispCN"].get<std::string>());
                            result.push_back(data["ispCN"]);
                        }
                        logger.info(
                            "result:" + result[0] + " " + result[1] + " " + result[2] + " " + result[3] + " "
                            + result[4]
                        );

                        safeCallback(callback, result, uuidA);
                    } catch (const std::exception& e) {
                        logger.error("Error processing JSON data: " + std::string(e.what()));
                        safeCallback(callback, {}, uuidA);
                    }
                }
            } else {
                safeCallback(callback, {}, uuidA);
            }
        },
        3000
    );
}

bool displayNotice(mce::UUID& uuid) {
    auto level = ll::service::getLevel();
    if (!level.has_value()) return false;
    auto* player = level->getPlayer(uuid);
    if (!player) return false;
    if (!playerData.playerConfigs[uuid].displayOthers) return false;
    auto& playerConfig     = playerData.playerConfigs[uuid];
    auto& playerName       = playerConfig.realName;
    auto& playerDeviceName = playerConfig.deviceName;
    auto& playerLocation   = playerConfig.location;
    level->forEachPlayer([&](Player& player) -> bool {
        if (!playerData.playerConfigs[player.getUuid()].disabledNotice) {
            logger.info("Display notice to {}", player.getRealName());
            if (config.enabledToast) {
                ToastRequestPacket pkt = ToastRequestPacket(
                    fmt::format("§e{}加入了服务器~", playerName),
                    fmt::format(
                        "§e欢迎来自 §c{}§b{}§g{}§5{}§7({}) §e的§4{}§e玩家§a {} §e 加入服务器~",
                        playerLocation[0],
                        playerLocation[1],
                        playerLocation[2],
                        playerLocation[3],
                        playerLocation[4],
                        playerDeviceName,
                        playerName
                    )
                );
                player.sendNetworkPacket(pkt);
            }
            if (config.enabledChat) {
                player.sendMessage(fmt::format(
                    "§e欢迎来自 §c{}§b{}§g{}§5{}§7({}) §e的§4{}§e玩家§a {} §e 加入服务器~",
                    playerLocation[0],
                    playerLocation[1],
                    playerLocation[2],
                    playerLocation[3],
                    playerLocation[4],
                    playerDeviceName,
                    playerName
                ));
            }
            return true;
        }
        return false;
    });
    return true;
}