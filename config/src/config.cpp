#include "config.hpp"
#include <nlohmann/json.hpp>
#include <fstream>


namespace {
    nlohmann::json load_json(const std::string& path) {
        std::ifstream f(path);
        nlohmann::json j;
        f >> j;
        return j;
    }

    config::ServerConfig g_server_config;
}

struct ServerConfig
{
    public:
    static ServerConfig& getInstance() noexcept;
    

    private:
    static ServerConfig instance;

};


