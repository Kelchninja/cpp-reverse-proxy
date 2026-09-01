#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>


namespace config
{
    // struct ServerConfig
    // {
    //     public:
    //     static ServerConfig& getInstance() noexcept
    //     {
    //         static ServerConfig instance;
    //         return instance;
    //     }


    //     struct Settings
    //     {
    //         size_t max_threads;
    //         bool max_performance_enabled;
    //     } settings;

    //     ServerConfig(const ServerConfig&) = delete;
    //     ServerConfig& operator=(const ServerConfig&) = delete;
    //     ServerConfig(ServerConfig&&) = delete;
    //     ServerConfig& operator=(ServerConfig&&) = delete;

    //     private:
    //     ServerConfig() = default;
    // };


    struct ServerConfig
    {
        public:
        ServerConfig(ServerConfig&) = delete;
        ServerConfig(ServerConfig&&) = delete;

        static void load(const std::filesystem::path&);
        static ServerConfig& getInstance();
        struct settings
        {
            bool enable_max_performance;
            size_t max_concurrent_threads;
        } settings;
        private:
        explicit ServerConfig(const nlohmann::json&);
        static ServerConfig instance;
    };
}
