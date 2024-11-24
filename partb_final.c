#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int COMMAND_SIZE = 5000;
int WD_SIZE = 10000;

void execute_command(char **token_all, int no_tokens, int input_fd, int output_fd,int flag ) {
    pid_t child_pid = fork();

    if (child_pid == -1) {
        perror("Fork failed:");
        exit(1);
    }

    if (child_pid == 0) {
        if (input_fd != STDIN_FILENO) {
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        if (output_fd != STDOUT_FILENO) {
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        }

        if (execvp(token_all[0], token_all) == -1) {
            perror("exec failed");
            exit(1);
        }
    } else {
        int status;
        if(flag)
        waitpid(child_pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Command failed with exit code %d\n", WEXITSTATUS(status));
            exit(1);
        }
    }
}

int main() {
    while (1) {
        printf("shell>");

        char command[COMMAND_SIZE];

        if (fgets(command, sizeof(command), stdin) == NULL) {
            perror("fgets failed");
            exit(1);
        }

        command[strcspn(command, "\n")] = '\0'; // Remove the trailing newline

        if (strcmp(command, "exit") == 0) {
            exit(0);
        }

        char *subcommands[COMMAND_SIZE];
        char *pipe_str = NULL;
        int no_pipes = 0;
        pipe_str = strtok(command, "|");
        subcommands[no_pipes++] = pipe_str;

        while ((pipe_str = strtok(NULL, "|")) != NULL) {
            subcommands[no_pipes++] = pipe_str;
        }


        if(no_pipes == 1)
        {
                char *token_all[COMMAND_SIZE];
                char *token = strtok(subcommands[0], " ");
                int no_tokens = 0;
                while (token != NULL) {
                    token_all[no_tokens++] = token;
                    token = strtok(NULL, " ");
                }
                int len_and = strlen(token_all[no_tokens - 1]);
                //printf("%d ",len_and);
                int flag =  (token_all[no_tokens - 1][len_and - 1] != '&');
            if(strcmp(token_all[0],"cd") == 0)
            {
                char wd[WD_SIZE];
                if (getcwd(wd, sizeof(wd)) != NULL) //get the current working directory 
                {
                        int lenwd = 0;
                        for(;wd[lenwd]!='\0';lenwd++);
                        wd[lenwd++] ='/';
                        for(int i = 1;i <no_tokens;i++)
                        {
                            int len = strlen(token_all[i]);

                            for(int j = 0 ; j < len ; j++)
                            {
                                wd[lenwd++] = token_all[i][j];
                            }
                        }

                        wd[lenwd] = '\0';// add the path of dir you want to go to at the end 

                        if(chdir(wd) == 0)// If change dir is successful
                        {
                            printf("Changed dir\n");

                        }
                        else// if change dir is not sucessful
                        {
                            perror("chdir:");
                        }

                        } 
                        else // if we cannot get the current working directory
                        {
                                perror("getcwd");
                                return 1;
                        }
                        continue;
            }
            else if(strcmp(token_all[0],"help") == 0)
            {
                 printf("pwd\ncd <dir name>\nmkdir <dir name>\nls <flag>\nexit\nhelp\n");
                 continue;
            }
            else
            {
                //printf("%s",token_all[0]);
                
                execute_command(token_all,no_tokens,STDIN_FILENO,STDOUT_FILENO,flag);
            }
            //break;
        }
        else
        {
            int input_fd = STDIN_FILENO;
            int i;
            for (i = 0; i < no_pipes - 1; i++) {


                int pipe_fds[2];
                if (pipe(pipe_fds) < 0) {
                    perror("Pipe error");
                    exit(1);
                }

                char *token_all[COMMAND_SIZE];
                char *token = strtok(subcommands[i], " ");
                int no_tokens = 0;
                while (token != NULL) {
                    token_all[no_tokens++] = token;
                    token = strtok(NULL, " ");
                }
                 int len_and = strlen(token_all[no_tokens - 1]);
                //printf("%d ",len_and);
                int flag =  (token_all[no_tokens - 1][len_and - 1] != '&');

                execute_command(token_all, no_tokens, input_fd, pipe_fds[1],flag);
                close(pipe_fds[1]);
                input_fd = pipe_fds[0];
            }

            char *token_all[COMMAND_SIZE];
            char *token = strtok(subcommands[i], " ");
            int no_tokens = 0;
            while (token != NULL) {
                token_all[no_tokens++] = token;
                token = strtok(NULL, " ");
            }
            int len_and = strlen(token_all[no_tokens - 1]);
                //printf("%d ",len_and);
            int flag =  (token_all[no_tokens - 1][len_and - 1] != '&');
            execute_command(token_all, no_tokens, input_fd, STDOUT_FILENO,flag);
        }
        
    }

    return 0;
}
