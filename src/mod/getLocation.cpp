#include "Global.h"
#include "nlohmann/json_fwd.hpp"
#include <httplib.h>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
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
void safeCallback(const std::function<void(std::vector<std::string>)>& callback, const std::vector<std::string>& result) {
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    if (callback) {
        callback(result);
    } else {
        logger.error("Callback is nullptr!");
        return;
    }
}
void getLocation(std::string ip, const std::function<void(std::vector<std::string>)>& callback) {
    string url = "https://mesh.if.iqiyi.com/aid/ip/info?version=1.1.1&ip=" + ip;
    HttpGet(
        url,
        [callback](int code, std::string body) {
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
                        if (data.contains("provinceCN") && data["provinceCN"].is_string()) {
                            logger.info("provinceCN:" + data["provinceCN"].get<std::string>());
                            result.push_back(data["provinceCN"].get<std::string>());
                        }
                        if (data.contains("cityCN") && data["cityCN"].is_string()) {
                            logger.info("cityCN:" + data["cityCN"].get<std::string>());
                            result.push_back(data["cityCN"].get<std::string>());
                        }

                        if (data.contains("countyCN") && data["countyCN"].is_string()) {
                            std::string countyCN = data["countyCN"].get<std::string>();
                            logger.info("countyCN:" + countyCN);
                            if (countyCN != "*") {
                                result.push_back(countyCN);
                            } else {
                                if (data.contains("townCN") && data["townCN"].is_string()) {
                                    logger.info("townCN:" + data["townCN"].get<std::string>());
                                    result.push_back(data["townCN"].get<std::string>());
                                } else {
                                    result.push_back("*");
                                }
                            }
                        }

                        if (data.contains("ispCN") && data["ispCN"].is_string()) {
                            logger.info("ispCN:" + data["ispCN"].get<std::string>());
                            result.push_back(data["ispCN"]);
                        }
                        logger.info("result:" + result[0] + " " + result[1] + " " + result[2] + " " + result[3]);
                        
                        safeCallback(callback,result);
                    } catch (const std::exception& e) {
                        logger.error("Error processing JSON data: " + std::string(e.what()));
                        safeCallback(callback,{});
                    }
                }
            } else {
                safeCallback(callback,{});
            }
        },
        3000
    );
}
