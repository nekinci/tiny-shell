#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>



#define READ 0
#define WRITE 1
#define MAX_PROCESS_COMMAND 10
#define MAX_ARG 15


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
char** arg_list(char *line) {
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
    return arg_list;
}

void do_processes(char** p_list, int p_count) {

    pid_t pid = fork();
    int m_fd[2];
    pipe(m_fd);
    int status;
    if (pid < 0){
        perror("Fork failed");
    } else if (pid > 0){
        // Parent waits child processes
        wait(NULL);
        close(m_fd[WRITE]);
        char data[2048];
        read(m_fd[READ], data, 2048);
        printf("in parent: %s", data);
    } else {
        int fd[2];
        pipe(fd);
        pid_t c_pid = fork();
        if (c_pid == 0){
            close(fd[READ]);
            close(m_fd[READ]);
            close(m_fd[WRITE]);
            dup2(fd[WRITE], STDOUT_FILENO);
            close(fd[WRITE]);
            char **program_args = arg_list(p_list[0]);
            execv(program_args[0], program_args);
            perror("Hata");
        } else if(c_pid > 0) {
            wait(NULL);
            close(fd[WRITE]);
            close(m_fd[READ]);
            dup2(fd[READ], stdin);
            close(fd[READ]);
            dup2(m_fd[WRITE], stdout);
            close(m_fd[WRITE]);
            char **program_args = arg_list(p_list[1]);
            execv(program_args[0], program_args);
            perror("Hata");
        }
    }

}

#define BUF_SIZE 1024
void prompt_and_read_command(){

    while (true){
        printf("> ");
        char *line;
        size_t buf_size = BUF_SIZE;
        char** p_list = malloc(sizeof(char*) * MAX_PROCESS_COMMAND);
        getline(&line, &buf_size, stdin);
        int p_count = process_list(line, p_list);
        do_processes(p_list, p_count);
    }
}


int main() {


    prompt_and_read_command();
    return 0;
}
