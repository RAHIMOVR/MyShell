#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

static char *input_line = NULL;

void cleanup(void) {
    if (input_line != NULL) {
        free(input_line);
        input_line = NULL;
    }
}

void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\n$ ");
        fflush(stdout);
    }
}

void execute_command(char **argv) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("command not found");
        exit(1);
    } else {
        waitpid(pid, NULL, 0);
    }
}

int main(void) {
    signal(SIGINT, signal_handler);

    while (1) {
        printf("$ ");
        fflush(stdout);

        size_t bufsize = 0;
        ssize_t len = getline(&input_line, &bufsize, stdin);

        if (len == -1) {
            printf("\n");
            break;
        }

        if (input_line[len - 1] == '\n') {
            input_line[len - 1] = '\0';
        }

        if (strlen(input_line) == 0) {
            continue;
        }

        if (strcmp(input_line, "exit") == 0) {
            break;
        }

        char *args[100];
        int i = 0;
        args[i] = strtok(input_line, " \t");
        while (args[i] != NULL && i < 99) {
            i++;
            args[i] = strtok(NULL, " \t");
        }

        if (args[0] != NULL) {
            execute_command(args);
        }
    }

    cleanup();
    return 0;
}