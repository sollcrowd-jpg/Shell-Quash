#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>  // For kill() and signal handling
#include <sys/time.h>  // For process timeout and timers

#define MAX_COMMAND_LEN 1024
#define MAX_TOKENS 100

extern char **environ; // To access environment variables

// Signal handler for SIGINT (Ctrl+C)
void handle_sigint(int sig) {
    printf("\nCaught signal %d. Use 'exit' to quit.\n", sig);
    printf("> ");  // Reprint the prompt
    fflush(stdout);
}

// Signal handler for process timeout
void handle_timeout(int sig) {
    printf("\nProcess timed out after 10 seconds!\n");
    kill(getpid(), SIGKILL);  // Terminate the process after timeout
}

// Set a timer to limit process execution to 10 seconds
void set_timer() {
    struct itimerval timer;
    timer.it_value.tv_sec = 10;  // Set the timer to 10 seconds
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);  // Set the timer
    signal(SIGALRM, handle_timeout);  // Call handle_timeout after 10 seconds
}

// Print shell prompt
void print_prompt() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s> ", cwd);
    } else {
        perror("getcwd");
    }
}

// Tokenize the input command
int tokenize(char *input, char *tokens[]) {
    int i = 0;
    tokens[i] = strtok(input, " \n");
    while (tokens[i] != NULL && i < MAX_TOKENS - 1) {
        i++;
        tokens[i] = strtok(NULL, " \n");
    }
    return i;
}

// Handle the 'cd' command
void handle_cd(char *path) {
    if (path == NULL || chdir(path) != 0) {
        perror("cd");
    }
}

// Handle the 'pwd' command
void handle_pwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
}

// Handle the 'echo' command
void handle_echo(char *tokens[], int token_count) {
    int i;
    for (i = 1; i < token_count; i++) {
        if (tokens[i][0] == '$') {
            char *env_value = getenv(tokens[i] + 1);  // Skip the '$'
            if (env_value) {
                printf("%s ", env_value);
            } else {
                printf(" ");
            }
        } else {
            printf("%s ", tokens[i]);
        }
    }
    printf("\n");
}

// Handle the 'env' command
void handle_env() {
    char **env;
    for (env = environ; *env != 0; env++) {
        printf("%s\n", *env);
    }
}

// Handle the 'setenv' command
void handle_setenv(char *tokens[]) {
    if (tokens[1] && tokens[2]) {
        if (setenv(tokens[1], tokens[2], 1) != 0) {
            perror("setenv");
        }
    } else {
        fprintf(stderr, "setenv: missing arguments\n");
    }
}

// Handle I/O redirection
int handle_io_redirection(char *tokens[], int token_count) {
    int i;
    for (i = 0; i < token_count; i++) {
        if (strcmp(tokens[i], ">") == 0) {
            int fd = open(tokens[i + 1], O_CREAT | O_WRONLY, 0644);
            if (fd == -1) {
                perror("open");
                return -1;
            }
            dup2(fd, STDOUT_FILENO);  // Redirect stdout to the file
            close(fd);
            tokens[i] = NULL;  // Remove redirection part from tokens
            return 1;
        }
        if (strcmp(tokens[i], "<") == 0) {
            int fd = open(tokens[i + 1], O_RDONLY);
            if (fd == -1) {
                perror("open");
                return -1;
            }
            dup2(fd, STDIN_FILENO);  // Redirect stdin from the file
            close(fd);
            tokens[i] = NULL;  // Remove redirection part from tokens
            return 1;
        }
    }
    return 0;
}

// Execute external commands by forking a process
void execute_external_command(char *tokens[], int background) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        handle_io_redirection(tokens, MAX_TOKENS);
        if (execvp(tokens[0], tokens) == -1) {
            perror("execvp");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork");
    } else {
        if (!background) {
            wait(NULL);  // Wait for child to finish if not background process
        } else {
            printf("Running in background [pid %d]\n", pid);
        }
    }
}

int main() {
    char input[MAX_COMMAND_LEN];
    char *tokens[MAX_TOKENS];
    
    signal(SIGINT, handle_sigint);  // Handle Ctrl+C (SIGINT)

    while (1) {
        print_prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) {
            perror("fgets");
            exit(1);
        }

        int token_count = tokenize(input, tokens);
        if (token_count == 0) {
            continue;  // Ignore empty input
        }

        // Check if command is a background process ('&')
        int background = 0;
        if (strcmp(tokens[token_count - 1], "&") == 0) {
            tokens[token_count - 1] = NULL;  // Remove '&'
            background = 1;
        }

        // Handle built-in commands
        if (strcmp(tokens[0], "exit") == 0) {
            break;  // Exit the shell
        } else if (strcmp(tokens[0], "cd") == 0) {
            handle_cd(tokens[1]);
        } else if (strcmp(tokens[0], "pwd") == 0) {
            handle_pwd();
        } else if (strcmp(tokens[0], "echo") == 0) {
            handle_echo(tokens, token_count);
        } else if (strcmp(tokens[0], "env") == 0) {
            handle_env();
        } else if (strcmp(tokens[0], "setenv") == 0) {
            handle_setenv(tokens);
        } else {
            set_timer();  // Set a 10-second timer for long-running processes
            execute_external_command(tokens, background);
        }
    }

    return 0;
}
