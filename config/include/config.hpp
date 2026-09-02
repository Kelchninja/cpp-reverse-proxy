#pragma once
#include <nlohmann/json.hpp>
#include <filesystem>


namespace config
{
    struct ServerConfig
    {
        const bool auto_detect_cores;
        const size_t available_cores;

        static ServerConfig load(const std::filesystem::path&);
        
        private:
        explicit ServerConfig(const nlohmann::json&);
    };
}
