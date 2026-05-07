#include "server.h"
#include "server_config.h"

int main(int argc, char const *argv[])
{
    server_start(PORT);
    return 0;
}
