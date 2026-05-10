#include "client.h"

int main(int argc, char const *argv[])
{
    if (argc < 2)
    {
        printf("Insert username\n");
        return 0;
    }

    client_connect(argv[1]);
    return 0;
}
