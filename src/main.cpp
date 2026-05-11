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

using klein::game::Game;

int main(int argc, char* argv[])
{
	(void)argc, (void)argv;

    std::print("{}", ASCII_ART);

#ifndef NDEBUG
    spdlog::set_level(spdlog::level::debug);
    spdlog::enable_backtrace(32);
#endif

    Game app{};
    app.run();

    return 0;
}

//#if defined(_WIN32)
//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>
//int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
//    return main(__argc, __argv);
//}
//#endif
