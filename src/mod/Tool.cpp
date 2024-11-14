#pragma once
#include "Global.h"
#include <ll/api/chrono/GameChrono.h>
#include <ll/api/schedule/Scheduler.h>
#include <ll/api/schedule/Task.h>
#include <ll/api/utils/StringUtils.h>
#include <regex>
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
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Iphlpapi.lib")
// 定义 DNS 报文头部结构体
#pragma pack(push, 1)
struct DNS_HEADER {
    unsigned short id;         // 16位 ID 字段
    unsigned short flags;      // 16位标志字段
    unsigned short q_count;    // 问题数
    unsigned short ans_count;  // 回答数
    unsigned short auth_count; // 权威记录数
    unsigned short add_count;  // 附加记录数
};
#pragma pack(pop)

// 定义 DNS 问题部分结构体
#pragma pack(push, 1)
struct QUESTION {
    unsigned short qtype;  // 查询类型
    unsigned short qclass; // 查询类别
};
#pragma pack(pop)

// 定义 DNS 回答部分结构体
#pragma pack(push, 1)
struct R_DATA {
    unsigned short type;
    unsigned short _class;
    unsigned int   ttl;
    unsigned short data_len;
    union {
        unsigned char a[4];   // IPv4 地址
        unsigned char ptr[1]; // 指针
    } rdata;
};
#pragma pack(pop)
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
    if (device.count(deviceName) == 0) {
        return "未知设备";
    } else {
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

bool isIP(const std::string& ip){
    std::regex reg(R"(/^((\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.){3}(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])(?::(?:[0-9]|[1-9][0-9]{1,3}|[1-5][0-9]{4}|6[0-4][0-9]{3}|65[0-4][0-9]{2}|655[0-2][0-9]|6553[0-5]))?$/)");
    return std::regex_match(ip, reg);
}

std::string resolvesDomain(const std::string& domain) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        logger.error("WSAStartup failed");
        return "";
    }

    // 创建一个 UDP 套接字
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        logger.error("socket failed");
        WSACleanup();
        return "";
    }

    // 设置 DNS 服务器
    sockaddr_in dnsServer;
    dnsServer.sin_family = AF_INET;
    dnsServer.sin_port = htons(53);
    inet_pton(AF_INET, "223.5.5.5", &dnsServer.sin_addr);

    // 构建 DNS 查询报文
    unsigned char queryBuffer[512];
    memset(queryBuffer, 0, sizeof(queryBuffer));
    DNS_HEADER* dnsHeader = (DNS_HEADER*)queryBuffer;
    dnsHeader->id = htons(12345); // 随机ID
    dnsHeader->flags = 0x0100; // 标准查询，递归请求
    dnsHeader->q_count = htons(1); // 一个问题

    // 添加问题部分
    unsigned char* p = queryBuffer + sizeof(DNS_HEADER);
    const char* domainPtr = domain.c_str();
    while (*domainPtr) {
        *p++ = strlen(domainPtr);
        while (*domainPtr) {
            *p++ = *domainPtr++;
        }
        domainPtr++;
    }
    *p++ = 0; // 结束标志
    QUESTION* question = (QUESTION*)p;
    question->qtype = htons(1); // 类型 A
    question->qclass = htons(1); // 类 IN
    p += sizeof(QUESTION);

    // 发送 DNS 查询
    int sent = sendto(sock, (char*)queryBuffer, p - queryBuffer, 0, (sockaddr*)&dnsServer, sizeof(dnsServer));
    if (sent == SOCKET_ERROR) {
        logger.error("sendto failed: " + std::to_string(WSAGetLastError()));
        closesocket(sock);
        WSACleanup();
        return "";
    }

    // 接收 DNS 响应
    unsigned char responseBuffer[1024];
    sockaddr_in from;
    int fromLen = sizeof(from);
    int received = recvfrom(sock, (char*)responseBuffer, sizeof(responseBuffer), 0, (sockaddr*)&from, &fromLen);
    if (received == SOCKET_ERROR) {
        logger.error("recvfrom failed: " + std::to_string(WSAGetLastError()));
        closesocket(sock);
        WSACleanup();
        return "";
    }

    // 解析 DNS 响应
    DNS_HEADER* responseHeader = (DNS_HEADER*)responseBuffer;
    if (ntohs(responseHeader->ans_count) > 0) {
        unsigned char* p = responseBuffer + sizeof(DNS_HEADER);
        // 跳过问题部分
        while (*p) {
            p += *p + 1;
        }
        p += sizeof(QUESTION); // 跳过类型和类

        // 解析答案部分
        for (int i = 0; i < ntohs(responseHeader->ans_count); ++i) {
            // 跳过名称
            while (*p & 0xC0) {
                p += 2; // 跳过指针
            }
            while (*p) {
                p += *p + 1;
            }
            p++; // 跳过结束标志

            R_DATA* rData = (R_DATA*)p;
            if (ntohs(rData->type) == 1 && ntohs(rData->data_len) == 4) {
                in_addr ip;
                memcpy(&ip, rData->rdata.a, 1);
                closesocket(sock);
                WSACleanup();
                return inet_ntoa(ip);
            }
            p += 10 + ntohs(rData->data_len); // 跳过类型、类、TTL、数据长度和数据
        }
    }

    closesocket(sock);
    WSACleanup();
    return "";
}