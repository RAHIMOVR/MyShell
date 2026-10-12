#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>

static char *input_line = NULL;

static size_t allocated_memory = 0;

void *ram_malloc(size_t size) {
    void *ptr = malloc(size);

    if (ptr != NULL) {
        allocated_memory += size;
    }

    return ptr;
}

void ram_free(void *ptr, size_t size) {
    if (ptr != NULL) {
        free(ptr);

        if (allocated_memory >= size)
            allocated_memory -= size;
    }
}

void show_memory_usage(void) {
    printf("Memory currently allocated: %zu bytes\n", allocated_memory);
}

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
            strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "help") == 0);
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
            if (argv[i + 1] != NULL) printf(" ");
        }
        printf("\n");
        return;
    }
    if (strcmp(argv[0], "help") == 0) {
        printf("Available Commands:\n");
        printf("  exit              - Close the shell\n");
        printf("  cd [dir]          - Change directory\n");
        printf("  echo [text]       - Print text to screen\n");
        printf("  help              - Show this help message\n");
        printf("\nOperators:\n");
        printf("  > file            - Redirect output to file\n");
        printf("  < file            - Redirect input from file\n");
        printf("  cmd1 | cmd2       - Pipe output of cmd1 to cmd2\n");
        printf("  cmd1 && cmd2      - Run cmd2 only if cmd1 succeeds\n");
        printf("  cmd1 || cmd2      - Run cmd2 only if cmd1 fails\n");
        printf("\nVariables:\n");
        printf("  $VAR              - Expand environment variable\n");
        printf("\nControls:\n");
        printf("  Ctrl-C            - Interrupt current command\n");
        printf("  Ctrl-D            - Exit shell\n");
        return;
    }
}

char *expand_variables(char *input) {
    size_t memory_size = strlen(input) * 4 + 1;

    char *result = ram_malloc(memory_size);

    if (!result) return NULL;

    int j = 0;
    for (int i = 0; input[i]; i++) {
        if (input[i] == '$') {
            int start = i + 1;
            while (input[start] && (isalnum(input[start]) || input[start] == '_')) start++;
            if (start > i + 1) {
                char varname[256] = {0};
                strncpy(varname, input + i + 1, start - i - 1);
                char *value = getenv(varname);
                if (value) {
                    for (int k = 0; value[k]; k++) result[j++] = value[k];
                }
                i = start - 1;
                continue;
            }
        }
        result[j++] = input[i];
    }
    result[j] = '\0';
    return result;
}

void setup_redirection(char **argv) {
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], ">") == 0 && argv[i + 1]) {
            int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, 1);
            close(fd);
            argv[i] = NULL;
            break;
        }
        if (strcmp(argv[i], "<") == 0 && argv[i + 1]) {
            int fd = open(argv[i + 1], O_RDONLY);
            if (fd < 0) { perror("open"); exit(1); }
            dup2(fd, 0);
            close(fd);
            argv[i] = NULL;
            break;
        }
    }
}

int has_pipe(char **argv) {
    for (int i = 0; argv[i]; i++)
        if (strcmp(argv[i], "|") == 0) return i;
    return -1;
}

void execute_pipe(char **argv, int pipe_pos) {
    int pipefd[2];
    if (pipe(pipefd) < 0) { perror("pipe"); return; }

    argv[pipe_pos] = NULL;
    char **right_args = argv + pipe_pos + 1;

    pid_t pid1 = fork();
    if (pid1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], 1);
        close(pipefd[1]);
        setup_redirection(argv);
        if (!is_builtin(argv[0])) {
            execvp(argv[0], argv);
            perror("command not found");
        } else {
            exec_builtin(argv);
        }
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        close(pipefd[1]);
        dup2(pipefd[0], 0);
        close(pipefd[0]);
        setup_redirection(right_args);
        if (!is_builtin(right_args[0])) {
            execvp(right_args[0], right_args);
            perror("command not found");
        } else {
            exec_builtin(right_args);
        }
        exit(1);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

int has_logical(char **argv, char **op) {
    for (int i = 0; argv[i]; i++) {
        if (strcmp(argv[i], "&&") == 0 || strcmp(argv[i], "||") == 0) {
            *op = argv[i];
            argv[i] = NULL;
            return i;
        }
    }
    return -1;
}

int run_command(char **argv) {
    if (!argv[0]) return 0;

    int saved_stdin = dup(0), saved_stdout = dup(1);
    setup_redirection(argv);

    int status = 0;
    if (is_builtin(argv[0])) {
        exec_builtin(argv);
        status = 0;
    } else {
        pid_t pid = fork();
        if (pid == 0) {
            execvp(argv[0], argv);
            perror("command not found");
            exit(127);
        }
        waitpid(pid, &status, 0);
        status = WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }

    dup2(saved_stdin, 0);
    dup2(saved_stdout, 1);
    close(saved_stdin);
    close(saved_stdout);
    return status;
}

void execute_command(char **argv) {
    if (!argv[0]) return;

    int pipe_pos = has_pipe(argv);
    if (pipe_pos >= 0) {
        execute_pipe(argv, pipe_pos);
        return;
    }

    char *op = NULL;
    int log_pos = has_logical(argv, &op);
    if (log_pos >= 0) {
        char **right_args = argv + log_pos + 1;
        int status = run_command(argv);
        if ((strcmp(op, "&&") == 0 && status == 0) ||
            (strcmp(op, "||") == 0 && status != 0)) {
            run_command(right_args);
        }
        return;
    }

    run_command(argv);
}

int main(void) {
    signal(SIGINT, signal_handler);

    while (1) {
        printf("$ ");
        fflush(stdout);

        size_t bufsize = 0;
        ssize_t len = getline(&input_line, &bufsize, stdin);
        if (len == -1) { printf("\n"); break; }
        if (input_line[len-1] == '\n') input_line[len-1] = '\0';
        if (strlen(input_line) == 0) continue;

        char *expanded = expand_variables(input_line);
        if (!expanded) continue;

        char *args[100];
        int i = 0;
        args[i] = strtok(expanded, " \t");
        while (args[i] && i < 99) {
            i++;
            args[i] = strtok(NULL, " \t");
        }

        if (args[0]) execute_command(args);

        ram_free(expanded, strlen(input_line) * 4 + 1);
    }

    cleanup();

    show_memory_usage();

    return 0;
}
