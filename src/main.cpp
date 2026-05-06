#include "klein/game.hpp"

#include <print>
#include <spdlog/spdlog.h>

const char* ASCII_ART = R"( _    _      _
| |  | |    (_)
| | _| | ___ _ _ __
| |/ / |/ _ \ | '_ \
|   <| |  __/ | | | |
|_|\_\_|\___|_|_| |_|
)";

int main(int argc, char* argv[])
{
    std::print("{}", ASCII_ART);

#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
    spdlog::enable_backtrace(32);
#endif

    klein::Game app{};
    app.run();

    return 0;
}
