#include "functions.h"
#include "test.h"

int main(int argc, char *argv[]) {
    Disc_conf config;

    if (checkPort(config.port_poole) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid poole port.\n");
        printF(C_RESET);
        return -1;
    }

    if (checkPort(config.port_bow) == -1) {
        printF(C_BOLDRED);
        printF("ERROR: Invalid bow port.\n");
        printF(C_RESET);
        return -1;
    }

    return 0;
}