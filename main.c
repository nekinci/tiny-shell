#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <pwd.h>
#include "util.h"

#define READ 0
#define WRITE 1
#define MAX_PROCESS_COMMAND 10
#define MAX_ARG 15

typedef struct {
    char **argv;
    int argc;
    char* cmd;
    bool is_builtin;
} Process;

const char* builtin_command_list[] = {"cd", "help", "exit"};

#define PIPE_DELIM "|\n"
int process_list(char*line, char ** p_list){
    char* delim = PIPE_DELIM;
    int p = 0;
    char* token = strtok(line, delim);
    p_list[p] = token;
    while (token != NULL){
        p++;
        token = strtok(NULL, delim);
        p_list[p] = token;
    }

    return p;
}

#define ARG_DELIM " \t\r\n\a"
char** arg_list(char *line, int *arg_count) {
    char *delim = ARG_DELIM;
    int argc = 0;
    char** arg_list = malloc(sizeof(char*) * MAX_ARG);
    char* token = strtok(line, delim);
    arg_list[argc++] = token;
    while (token != NULL){
        token = strtok(NULL, delim);
        arg_list[argc++] = token;
    }
    arg_list[argc++] = NULL;
    *arg_count = argc;
    return arg_list;
}


bool is_builtin(char* cmd){
    int index = 0;
    while (builtin_command_list[index] != NULL)
        if (strcmp(builtin_command_list[index++], cmd))
            return true;
    return false;
}

void built_in_command(char **p_list, int p_count) {

}


Process** group_processes(char **p_list, int p_count){
    Process** processes = malloc(sizeof(Process*) * p_count);
    for (int i = 0; i < p_count; i++){
        Process *p = malloc(sizeof(Process));
        int argc;
        char **argv = arg_list(p_list[i], &argc);

        if (argv[0] != NULL){
            trim(argv[0]);
        }

        p -> is_builtin = is_builtin(argv[0]);
        p -> argv = argv;
        p -> argc = argc;
        p -> cmd = argv[0];
        processes[i] = p;
    }

    return processes;
}



void do_processes_helper(Process** p_list, int p_count){

    int parent_pid = fork();
    if (parent_pid > 0) {
        wait(NULL);
    } else if (parent_pid == 0){
        int fd[2];
        pid_t pid;
        int before_in = STDIN_FILENO;

        for (int i = 0; i < p_count; i++){
            pipe(fd);
            pid = fork();
            if (pid < 0){
                perror("An error occured while trying to fork process");
                exit(EXIT_FAILURE);
            } else if (pid > 0){
                // It's parent process and waits the own child process.
                close(fd[WRITE]);
                wait(NULL);
                before_in = fd[READ];
            } else {
                close(fd[READ]);
                dup2(fd[WRITE], STDOUT_FILENO);
                close(fd[WRITE]);
                // # process before input file descriptoru ile stdinputu yer degistirir ve böylece bir önceki processten gelen result
                // process için bir input niteliği taşır.
                dup2(before_in, STDIN_FILENO);
                close(before_in);
                execvp(p_list[i]->cmd, p_list[i] -> argv);
                exit(0);
            }
        }

        char c;
        char *data = malloc(sizeof(c) * BUFSIZ);
        int i = 0;
        while (read(before_in, &c, sizeof(char) * 1) > 0){
            data[i++] = c;
            if (strlen(data) == BUFSIZ){
                char *new_str = realloc(data, strlen(data) * sizeof(char) * 2);
                data = new_str;
            }
        }
        close(before_in);
        printf("%s", data);
        exit(0);

    } else {
        perror("Process failed");
    }
}

void do_processes(char **p_list, int p_count){
    Process** process_list = group_processes(p_list, p_count);
    do_processes_helper(process_list, p_count);
}


char* prompt_current_dir(){
    char cd[BUFSIZ];
    getcwd(cd, sizeof(cd));
    char *ret = malloc(sizeof(char) * BUFSIZ);
    strcpy(ret, cd);
    return ret;
}

char* prompt_message() {
    struct passwd *pw = getpwuid(geteuid());
    return pw -> pw_name;
}

#define BUF_SIZE 1024
void prompt(){

    while (true){
        printf("%s %s $ ", prompt_message(), prompt_current_dir());
        char *line;
        size_t buf_size = BUF_SIZE;
        char** p_list = malloc(sizeof(char*) * MAX_PROCESS_COMMAND);
        getline(&line, &buf_size, stdin);
        int p_count = process_list(line, p_list);
        do_processes(p_list, p_count);
    }
}


int main() {
    prompt();
    return 0;
}
