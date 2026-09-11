#include "proxy.hpp"
#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
    using Headers = std::vector<std::pair<std::string, std::string>>;

    constexpr std::string_view via_token{"1.1 httpserver"};

    constexpr std::array<std::string_view, 8> hop_by_hop{
        "connection",
        "keep-alive",
        "proxy-authenticate",
        "proxy-authorization",
        "te",
        "trailer",
        "transfer-encoding",
        "upgrade"
    };

    std::string* find_header(Headers& headers, std::string_view name)
    {
        auto it{std::ranges::find_if(headers, [name](const auto& h) { return h.first == name; })};
        return it == headers.end() ? nullptr : &it->second;
    }

    void remove_header(Headers& headers, std::string_view name)
    {
        std::erase_if(headers, [name](const auto& h) { return h.first == name; });
    }

    void set_header(Headers& headers, std::string_view name, std::string value)
    {
        remove_header(headers, name);
        headers.emplace_back(std::string{name}, std::move(value));
    }

    void append_header(Headers& headers, std::string_view name, std::string_view value)
    {
        if (auto* existing{find_header(headers, name)})
        {
            existing->append(", ").append(value);
            return;
        }
        headers.emplace_back(std::string{name}, std::string{value});
    }

    std::string normalize_token(std::string_view token)
    {
        auto begin{token.find_first_not_of(" \t")};
        auto end{token.find_last_not_of(" \t")};
        if (begin == std::string_view::npos) return {};

        std::string out{token.substr(begin, end - begin + 1)};
        std::ranges::transform(out, out.begin(), [](unsigned char c) { return std::tolower(c); });
        return out;
    }

    void strip_hop_by_hop(Headers& headers)
    {
        std::vector<std::string> listed;
        if (const auto* connection{find_header(headers, "connection")})
        {
            std::string_view rest{*connection};
            while (!rest.empty())
            {
                auto comma{rest.find(',')};
                listed.push_back(normalize_token(rest.substr(0, comma)));
                if (comma == std::string_view::npos) break;
                rest.remove_prefix(comma + 1);
            }
        }

        std::erase_if(headers, [&listed](const auto& h) {
            return std::ranges::contains(hop_by_hop, h.first)
                || std::ranges::contains(listed, h.first);
        });
    }

    void set_content_length(Headers& headers, size_t body_size)
    {
        set_header(headers, "content-length", std::to_string(body_size));
    }
}


namespace gateway
{
    void prepare_upstream_request(http::HttpRequest& req, const asio::ip::tcp::endpoint& client)
    {
        std::string original_host;
        if (const auto* host{find_header(req.headers, "host")})
        {
            original_host = *host;
        }

        strip_hop_by_hop(req.headers);

        set_header(req.headers, "x-forwarded-for", client.address().to_string());
        set_header(req.headers, "x-forwarded-proto", "http");
        if (!original_host.empty())
        {
            set_header(req.headers, "x-forwarded-host", std::move(original_host));
        }
        append_header(req.headers, "via", via_token);

        set_content_length(req.headers, req.body.size());

        set_header(req.headers, "connection", "close");
    }

    void prepare_downstream_response(http::HttpResponse& res)
    {
        strip_hop_by_hop(res.headers);
        append_header(res.headers, "via", via_token);

        const bool no_content_length{res.status_code / 100 == 1
            || res.status_code == 204
            || res.status_code == 304};

        if (!no_content_length)
        {
            set_content_length(res.headers, res.body.size());
        }
    }
}
