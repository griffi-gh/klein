#include "klein/game.hpp"

int main(int argc, char* argv[])
{
    klein::Game app{};
    app.init();
    app.run();
    return 0;
}
