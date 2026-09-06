#include "config.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <nlohmann/json_fwd.hpp>
#include <stdexcept>

#define CONFIG_PATH "test_config.json"

static constexpr const char* DEFAULT_IPV4{"127.0.0.1"};
static constexpr uint16_t DEFAULT_PORT{8080};

// Schreibt exakt die uebergebenen Eintraege, ohne Defaults.
static nlohmann::json build_json_raw(const std::vector<std::pair<std::string, nlohmann::json>>& serverEntries)
{
    nlohmann::json j;

    for (const auto& [key, value] : serverEntries)
    {
        j["server"][key] = value;
    }

    j["security"] = nlohmann::json::object();
    j["processing"] = nlohmann::json::object();
    j["api"] = nlohmann::json::object();

    return j;
}

// Fuellt alle Pflichtfelder mit Defaults; uebergebene Eintraege ueberschreiben diese.
static nlohmann::json build_json(const std::vector<std::pair<std::string, nlohmann::json>>& overrides)
{
    std::vector<std::pair<std::string, nlohmann::json>> entries{
        {"auto_detect_cores", true},
        {"available_cores", 0},
        {"IPv4", DEFAULT_IPV4},
        {"port", DEFAULT_PORT}
    };

    entries.insert(entries.end(), overrides.begin(), overrides.end());

    return build_json_raw(entries);
}

static void write_config(const nlohmann::json& j)
{
    std::ofstream out{CONFIG_PATH};
    ASSERT_TRUE(out.is_open()) << "Konnte " << CONFIG_PATH << " nicht öffnen (cwd: "
                                << std::filesystem::current_path() << ")";
    out << j.dump(4);
    out.close();
    ASSERT_TRUE(std::filesystem::exists(CONFIG_PATH));
}

TEST(ConfigTest, ValidConfig_AutoDetectFalse)
{
    auto j = build_json({{"auto_detect_cores", false}, {"available_cores", 6}});
    write_config(j);

    std::optional<config::ServerConfig> config;
    ASSERT_NO_THROW(config.emplace(config::ServerConfig::load(CONFIG_PATH)));

    ASSERT_TRUE(config.has_value());
    EXPECT_FALSE(config->auto_detect_cores);
    EXPECT_EQ(config->available_cores, 6uz);
}

TEST(ConfigTest, ValidConfig_AutoDetectTrue_ZeroCores)
{
    auto j = build_json({{"auto_detect_cores", true}, {"available_cores", 0}});
    write_config(j);

    std::optional<config::ServerConfig> config;
    EXPECT_NO_THROW(config.emplace(config::ServerConfig::load(CONFIG_PATH)));

    ASSERT_TRUE(config.has_value());
    EXPECT_TRUE(config->auto_detect_cores);
    EXPECT_EQ(config->available_cores, 0uz);
}

TEST(ConfigTest, ValidConfig_NegativeCores_ClampedToZero)
{
    auto j = build_json({{"auto_detect_cores", true}, {"available_cores", -242}});
    write_config(j);

    std::optional<config::ServerConfig> config;
    EXPECT_NO_THROW(config.emplace(config::ServerConfig::load(CONFIG_PATH)));

    ASSERT_TRUE(config.has_value());
    EXPECT_TRUE(config->auto_detect_cores);
    EXPECT_EQ(config->available_cores, 0uz);
}

TEST(ConfigTest, NegativeCores_ForcesAutoDetectTrue)
{
    auto j = build_json({{"auto_detect_cores", false}, {"available_cores", -1}});
    write_config(j);

    std::optional<config::ServerConfig> config;
    EXPECT_NO_THROW(config.emplace(config::ServerConfig::load(CONFIG_PATH)));

    ASSERT_TRUE(config.has_value());
    EXPECT_TRUE(config->auto_detect_cores);
    EXPECT_EQ(config->available_cores, 0uz);
}

TEST(ConfigTest, ZeroCores_ForcesAutoDetectTrue)
{
    auto j = build_json({{"auto_detect_cores", false}, {"available_cores", 0}});
    write_config(j);

    std::optional<config::ServerConfig> config;
    EXPECT_NO_THROW(config.emplace(config::ServerConfig::load(CONFIG_PATH)));

    ASSERT_TRUE(config.has_value());
    EXPECT_TRUE(config->auto_detect_cores);
    EXPECT_EQ(config->available_cores, 0uz);
}

TEST(ConfigTest, MissingServerSection_Throws)
{
    auto j = build_json_raw({});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), std::runtime_error);
}

TEST(ConfigTest, MissingAvailableCores_Throws)
{
    auto j = build_json_raw({{"auto_detect_cores", false},
                             {"IPv4", DEFAULT_IPV4},
                             {"port", DEFAULT_PORT}});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), std::runtime_error);
}

TEST(ConfigTest, MissingAutoDetectCores_Throws)
{
    auto j = build_json_raw({{"available_cores", 6},
                             {"IPv4", DEFAULT_IPV4},
                             {"port", DEFAULT_PORT}});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), std::runtime_error);
}

// --- IPv4 / port ---

TEST(ConfigTest, ValidConfig_IPv4AndPort)
{
    auto j = build_json({{"IPv4", "192.168.178.42"}, {"port", 12345}});
    write_config(j);

    std::optional<config::ServerConfig> config;
    ASSERT_NO_THROW(config.emplace(config::ServerConfig::load(CONFIG_PATH)));

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->ipv4, "192.168.178.42");
    EXPECT_EQ(config->port, 12345);
}

TEST(ConfigTest, ValidConfig_DefaultsAreParsed)
{
    auto j = build_json({{"auto_detect_cores", false}, {"available_cores", 6}});
    write_config(j);

    std::optional<config::ServerConfig> config;
    ASSERT_NO_THROW(config.emplace(config::ServerConfig::load(CONFIG_PATH)));

    ASSERT_TRUE(config.has_value());
    EXPECT_EQ(config->ipv4, DEFAULT_IPV4);
    EXPECT_EQ(config->port, DEFAULT_PORT);
    EXPECT_FALSE(config->auto_detect_cores);
    EXPECT_EQ(config->available_cores, 6uz);
}

TEST(ConfigTest, ValidConfig_PortBoundaries)
{
    for (const uint16_t port : {uint16_t{1}, uint16_t{80}, uint16_t{65535}})
    {
        auto j = build_json({{"port", port}});
        write_config(j);

        std::optional<config::ServerConfig> config;
        ASSERT_NO_THROW(config.emplace(config::ServerConfig::load(CONFIG_PATH))) << "port " << port;

        ASSERT_TRUE(config.has_value());
        EXPECT_EQ(config->port, port);
    }
}

TEST(ConfigTest, MissingIPv4_Throws)
{
    auto j = build_json_raw({{"auto_detect_cores", false},
                             {"available_cores", 6},
                             {"port", DEFAULT_PORT}});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), std::runtime_error);
}

TEST(ConfigTest, MissingPort_Throws)
{
    auto j = build_json_raw({{"auto_detect_cores", false},
                             {"available_cores", 6},
                             {"IPv4", DEFAULT_IPV4}});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), std::runtime_error);
}

TEST(ConfigTest, IPv4WrongType_Throws)
{
    auto j = build_json({{"IPv4", 12345}});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), std::runtime_error);
}

TEST(ConfigTest, PortWrongType_Throws)
{
    auto j = build_json({{"port", "8080"}});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), std::runtime_error);
}