#include "config.hpp"
#include <cstdint>
#include <fstream>
#include <filesystem>
#include <nlohmann/detail/exceptions.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>
#include <iostream>
#include <string>


namespace
{
    // TODO: save parse into a static var, so it doesnt have to parse the same config every time.
    inline nlohmann::json load_json(const std::filesystem::path& path) {
        std::ifstream file{path};
        if (!file)
        {
            throw std::runtime_error("config.json not found " + path.string());
        }
        
        return nlohmann::json::parse(file);
    }

}

namespace config
{

    ServerConfig ServerConfig::load(const std::filesystem::path& path)
    {
        try
        {
            const auto j = load_json(path);
            const auto& head{j.at("server")};

            auto available_cores{std::max<int64_t>(head.at("available_cores").get<int64_t>(), 0)};
            const bool auto_detect{available_cores == 0
                || head.at("auto_detect_cores").get<bool>()};

            auto ipv4{head.at("IPv4").get<std::string>()};
            auto port{head.at("port").get<uint16_t>()};

            return ServerConfig{auto_detect, static_cast<size_t>(available_cores), std::move(ipv4), port};
        }
        catch (const nlohmann::json::exception& e)
        {
            throw std::runtime_error{"Failed to parse server config '" + path.string() + "': " + e.what()};
        }
    }

    ServerConfig::ServerConfig(bool auto_detect_cores, size_t available_cores, std::string ipv4, uint16_t port)
    : auto_detect_cores{auto_detect_cores}, available_cores{available_cores}, ipv4{ipv4}, port{port} {}
}

