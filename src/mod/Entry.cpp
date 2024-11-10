#include "mod/Entry.h"

#include "Entry.h"
#include "Global.h"

#include <ll/api/Config.h>
#include <ll/api/mod/RegisterHelper.h>
#include <memory>
join_location::Config config;
ll::Logger            logger("joinLocation");
namespace join_location {

static std::unique_ptr<Entry> instance;

Entry& Entry::getInstance() { return *instance; }

bool Entry::load() {
    getSelf().getLogger().debug("Loadding config...");
    auto path = getSelf().getConfigDir() / "config.json";
    try {
        ll::config::loadConfig(config, path);
        auto                   defaultConfig = join_location::PlayerConfig();
        std::vector<mce::UUID> removes;
        for (auto& item : config.playerConfigs) {
            if (item.second == defaultConfig) removes.push_back(item.first);
        }
        for (auto& item : removes) config.playerConfigs.erase(item);
    } catch (...) {}
    ll::config::saveConfig(config, path);
    logger.info(fmt::format(fmt::fg(fmt::color::pink), "模组JoinLocation重制版已加载！\n版本：{}", VERSION.to_string())
    );
    return true;
}

bool Entry::enable() {
    getSelf().getLogger().debug("Enabling...");
    ListenerCall(true);
    // registerCommands();
    // if (config.enabledPAPI) PapiCall(true);
    return true;
}

bool Entry::disable() {
    getSelf().getLogger().debug("Disabling...");
    ListenerCall(false);
    // if (config.enabledPAPI) PapiCall(false);
    //  Code for disabling the mod goes here.
    return true;
}

} // namespace join_location

LL_REGISTER_MOD(join_location::Entry, join_location::instance);
