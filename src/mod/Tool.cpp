#pragma once
#include "Global.h"
#include <string>
#include <vector>

extern std::pair<std::string,std::string> SplitIpPort(const std::string& ip_port) {
    std::string ip, port;
    size_t pos = ip_port.find(':');
    if (pos != std::string::npos) {
        ip = ip_port.substr(0, pos);
        port = ip_port.substr(pos + 1);
    } else {
        // 如果没有找到冒号，可以处理错误情况
        ip = ip_port;
        port = "";
    }
    return {ip, port};
}