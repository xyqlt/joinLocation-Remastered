#pragma once

#include <mc/deps/core/mce/UUID.h>
#include <string>
#include <unordered_map>
#include <vector>
namespace join_location {
struct PlayerConfig {
    bool                     disabledNotice = false; // 是否禁用通知
    bool                     displayOthers  = true; // 是否向他人显示通知
    bool                     isCached       = false; // 是否缓存
    std::string              ip             = "";   // IP
    std::string              realName       = "";    // 真实名称
    std::string              deviceName     = "";    // 设备名称
    std::vector<std::string> location; // 位置依次是[省份，地级市，县镇，运营商]
};
struct Config {
    int                                         version      = 1;              // 配置文件版本
    std::string                                 command      = "joinlocation"; // 命令名
    std::string                                 alias        = "location";     // 命令别名
    bool                                        enabledPAPI  = true;           // 注册PAPI变量
    bool                                        enabledToast = true;           // 显示通知
    bool                                        enabledChat  = true;           // 显示在聊天栏
    bool                                        enableCache  = true;           // 开启缓存
    
};
struct Data {
    int                                        version      = 1;                // 数据文件版本
    std::unordered_map<mce::UUID, PlayerConfig> playerConfigs;                 // 玩家配置
};
} // namespace join_location