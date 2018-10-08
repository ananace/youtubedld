#include "Server.hpp"

int main(int argc, const char** argv)
{
    Server srv;

    srv.init(argc, argv);
    srv.update();

    return 0;
}
