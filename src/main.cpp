#include "klein/game.hpp"
#include "spdlog/spdlog.h"

int main(int argc, char* argv[])
{
#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
    spdlog::enable_backtrace(32);
#endif

    klein::Game app{};
    app.run();

    return 0;
}
