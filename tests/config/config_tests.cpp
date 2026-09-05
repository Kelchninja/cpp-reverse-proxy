#include "config.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <nlohmann/json_fwd.hpp>

#define CONFIG_PATH "test_config.json"

static nlohmann::json build_json(const std::vector<std::pair<std::string, nlohmann::json>>& serverEntries)
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
    auto j = build_json({});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), nlohmann::json::out_of_range);
}

TEST(ConfigTest, MissingAvailableCores_Throws)
{
    auto j = build_json({{"auto_detect_cores", false}});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), nlohmann::json::out_of_range);
}

TEST(ConfigTest, MissingAutoDetectCores_Throws)
{
    auto j = build_json({{"available_cores", 6}});
    write_config(j);

    EXPECT_THROW(config::ServerConfig::load(CONFIG_PATH), nlohmann::json::out_of_range);
}