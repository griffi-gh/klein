#include "app.hpp"

int main(int argc, char* argv[])
{
    Game app{};
    app.init();
    app.run();
    return 0;
}
