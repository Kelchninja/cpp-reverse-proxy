#pragma once
#include <memory>
#include <llhttp.h>

namespace server
{


    class Connection : std::enable_shared_from_this<Connection>
    {
        public:
        protected:
        private:
        llhttp_t parser_;
        llhttp_settings_t settings_;


    };

}