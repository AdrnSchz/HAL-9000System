#include "functions.h"
#include "test.h"

User_conf config;

void readConfig(char* file) {
    int fd_config;
    char *buffer;

    fd_config = open(file, O_RDONLY);

    if (fd_config == -1) {
        asprintf(&buffer, "EROOR: %s not found.\n", file);
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
        printF("Usage: ./poole <config_file>\n");
        return -1;
    }

    readConfig(argv[1]);
    checkName(&config.user);
    testBowConf(config);

    asprintf(&buffer, "\n%s user initialized\n", config.user);
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

                asprintf(&buffer,  "%s connected to HAL 9000 system, welcome music lover!\n", config.user);
                printF(buffer);
                break;
            case 1:
                printF("Thanks for using HAL 9000, see you soon, music lover!\n");
                goto end;
                break;
            case 2:
                printF("There are no songs\n");
                break;
            case 3:
                printF("There are no playlists\n");
                break;
            case 4:
                printF("There are no songs to download\n");
                break;
            case 5:
                printF("There are no songs being downloaded\n");
                break;
            case 6:
                printF("There are no downloads to clear\n");
                break;
            case 7:
                printF("Unknown command.\n");
                break;
            default:
                printF("ERROR: Please input a valid command.\n");
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
