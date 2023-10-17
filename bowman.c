#include "functions.h"
#include "test.h"

User_conf config;

void readConfig(char* file) {
    int fd_config;
    char *buffer;

    fd_config = open(file, O_RDONLY);

    if (fd_config == -1) {
        asprintf(&buffer, C_BOLDRED "ERROR: %s not found.\n" C_RESET, file);
        printF(buffer);
        free(buffer);
        exit(-1);
    }

    else {
        readLine(fd_config, &config.user);
        readLine(fd_config, &config.files_path);
        readLine(fd_config, &config.ip);
        readNum(fd_config, &config.port);

        close(fd_config);
    }
}

void sig_handler(int sigsum) {

    switch(sigsum) {
        case SIGINT:
            printF("\nAborting...\n");
            free(config.user);
            free(config.files_path);
            free(config.ip);
            config.user = NULL;
            config.files_path = NULL;
            config.ip = NULL;
            exit(0);
            break;
    }
}

int main(int argc, char *argv[]) {
    char* buffer;
    signal(SIGINT, sig_handler);

    if (argc != 2) {
        printF(C_BOLDRED);
        printF("Usage: ./bowman <config_file>\n");
        printF(C_RESET);
        return -1;
    }

    readConfig(argv[1]);
    checkName(&config.user);
    //testBowConf(config);

    asprintf(&buffer, BOLD "\n%s user initialized\n" C_RESET, config.user);
    printF(buffer);
    free(buffer);
    buffer = NULL;
    while(1) {
        printF("$ ");
        readLine(0, &buffer);
        switch (checkCommand(buffer)) {
            case 0:
                free(buffer);
                buffer = NULL;

                asprintf(&buffer, C_GREEN "%s connected to HAL 9000 system, welcome music lover!\n" C_RESET, config.user);
                printF(buffer);
                break;
            case 1:
                printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                goto end;
                break;
            case 2:
                printF(C_RED);
                printF("There are no songs\n");
                printF(C_RESET);
                break;
            case 3:
                printF(C_RED);
                printF("There are no playlists\n");
                printF(C_RESET);
                break;
            case 4:
                printF(C_RED);
                printF("There are no songs to download\n");
                printF(C_RESET);
                break;
            case 5:
                printF(C_RED);
                printF("There are no songs being downloaded\n");
                printF(C_RESET);
                break;
            case 6:
                printF(C_RED);
                printF("There are no downloads to clear\n");
                printF(C_RESET);
                break;
            case 7:
                printF(C_BOLDRED);
                printF("Unknown command.\n");
                printF(C_RESET);
                break;
            default:
                printF(C_BOLDRED);
                printF("ERROR: Please input a valid command.\n");
                printF(C_RESET);
                break;
        }

        free(buffer);
        buffer = NULL;
    }

    end:
    free(buffer);
    free(config.user);
    free(config.files_path);
    free(config.ip);
    buffer = NULL;
    config.user = NULL;
    config.files_path = NULL;
    config.ip = NULL;

    return 0;
}
