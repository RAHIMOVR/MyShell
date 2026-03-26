#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>  // ADDED: For file opening flags

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

int is_builtin(char *cmd) {
    return (strcmp(cmd, "cd") == 0 || 
            strcmp(cmd, "echo") == 0 || 
            strcmp(cmd, "exit") == 0);
}

void exec_builtin(char **argv) {
    if (strcmp(argv[0], "exit") == 0) {
        exit(0);
    }
    
    if (strcmp(argv[0], "cd") == 0) {
        if (argv[1] == NULL) {
            fprintf(stderr, "cd: no argument\n");
        } else {
            if (chdir(argv[1]) != 0) {
                perror("cd");
            }
        }
        return;
    }
    
    if (strcmp(argv[0], "echo") == 0) {
        for (int i = 1; argv[i] != NULL; i++) {
            printf("%s", argv[i]);
            if (argv[i + 1] != NULL) {
                printf(" ");
            }
        }
        printf("\n");
        return;
    }
}

// ADDED: Helper to setup redirection and clean args
void setup_redirection(char **argv) {
    int i = 0;
    while (argv[i] != NULL) {
        if (strcmp(argv[i], ">") == 0) {
            // OUTPUT REDIRECTION
            int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, 1); // Replace stdout
            close(fd);   // Close original fd after dup2
            
            // Remove ">" and filename from args
            argv[i] = NULL; 
            break;
        }
        if (strcmp(argv[i], "<") == 0) {
            // INPUT REDIRECTION
            int fd = open(argv[i + 1], O_RDONLY);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, 0); // Replace stdin
            close(fd);   // Close original fd after dup2
            
            // Remove "<" and filename from args
            argv[i] = NULL;
            break;
        }
        i++;
    }
}

void execute_command(char **argv) {
    if (argv[0] == NULL) return;

    // ADDED: Save original stdin/stdout to restore later
    int saved_stdin = dup(0);
    int saved_stdout = dup(1);

    // ADDED: Check and setup redirection (modifies FDs)
    setup_redirection(argv);

    if (is_builtin(argv[0])) {
        // Builtins run in parent, so we must restore FDs after
        exec_builtin(argv);
        dup2(saved_stdin, 0);
        dup2(saved_stdout, 1);
        close(saved_stdin);
        close(saved_stdout);
        return;
    }
    
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return;
    }
    if (pid == 0) {
        // Child inherits redirected FDs from setup_redirection
        execvp(argv[0], argv);
        perror("command not found");
        exit(1);
    } else {
        waitpid(pid, NULL, 0);
        // Parent restores FDs after child finishes
        dup2(saved_stdin, 0);
        dup2(saved_stdout, 1);
        close(saved_stdin);
        close(saved_stdout);
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