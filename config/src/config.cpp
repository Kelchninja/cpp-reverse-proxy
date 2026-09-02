#include "config.hpp"
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <stdexcept>


namespace
{
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
        auto json = load_json(path);
        // sets the auto_detect flag, if someone sets a false core amount and doesnt have auto_detect on true
        if (json["available_cores"] <= 0) json["auto_detect_cores"] = true;
        ServerConfig config{json};
        return config;
    }

    ServerConfig::ServerConfig(const nlohmann::json& json)
    : auto_detect_cores(json["auto_detect_cores"]), 
    available_cores(json["available_cores"]) {}
}

