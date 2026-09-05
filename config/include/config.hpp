#pragma once
#include <cstdint>
#include <nlohmann/json.hpp>
#include <filesystem>


namespace config
{
    struct ServerConfig
    {
        const bool auto_detect_cores;
        const size_t available_cores;
        const std::string ipv4;
        const uint16_t port;

        static ServerConfig load(const std::filesystem::path&);
        
        private:
        ServerConfig(bool, size_t, std::string, uint16_t);
    };
}
