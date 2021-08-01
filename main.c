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

    int pipe_fd[2];
    pipe(pipe_fd);
    pid_t pid = fork();
    if (pid < 0){
        perror("Fork failed");
    } else if (pid > 0){
        // Parent waits child processes
        wait(NULL);
        char data[20];
        close(pipe_fd[1]);
        read(pipe_fd[0], data, sizeof(data));
        close(pipe_fd[0]);
        printf("in parent: %s\n", data);
    } else {

        int fd[2];
        pipe(fd);
        pid_t c_pid = fork();
        if (c_pid == 0){
            close(pipe_fd[WRITE]);
            close(pipe_fd[READ]);
            close(fd[READ]);
            dup2(fd[WRITE], STDOUT_FILENO);
            close(fd[WRITE]);
            char **program_args = arg_list(p_list[0]);
            execv(program_args[0], program_args);
            perror("Hata");
        } else if(c_pid > 0) {
            wait(NULL);
            char **program_args = arg_list(p_list[1]);
            dup2(fd[READ], STDIN_FILENO);  // Read pipe ucunu standart input dosyasını gösterecek şekilde yönlendirdik. (kopyaladık.)
            close(fd[WRITE]); // Yazma pipe ucunu kapattık cünkü kullanmayacagız.
            close(fd[READ]);

            // Parent process piping
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]);
            execv(program_args[0], program_args);
            perror("Hata");
            exit(0);
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
