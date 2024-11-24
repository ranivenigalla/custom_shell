#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
#include<ctype.h>
#include<ncurses.h>
#include <signal.h>
#include <pthread.h>

WINDOW *win;
char *filename;
int cursor_x, cursor_y;
int is_modified = 0;

int  COMMAND_SIZE = 5000;
int WD_SIZE = 10000;

void save_file() {
    FILE *file = fopen(filename, "w");
    if (file != NULL) {
        // Write your text content to the file
        // For simplicity, we'll assume the entire content is in the ncurses window
        for (int i = 0; i < LINES; i++) {
            char buffer[COLS];
            mvwinnstr(win, i, 0, buffer, COLS);
            fprintf(file, "%s\n", buffer);
        }
        fclose(file);
        is_modified = 0;
    }
}

void load_file() {
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        int ch;
        int row = 0, col = 0;

        while ((ch = fgetc(file)) != EOF) {
            if (ch == '\n') {
                // Move to the next row when encountering a newline
                row++;
                col = 0;
            } else if (row < LINES && col < COLS) {
                // Display the character in the window if it fits
                mvwaddch(win, row, col, ch);
                col++;
            }
        }

        fclose(file);
    }
}

void handle_exit() {
    //save_file();
    
    endwin();
    int num_lines = 0;
    int num_words = 0;
    int num_characters = 0;
    for (int i = 0; i < LINES; i++) 
    {
        char buffer[COLS];
        mvwinnstr(win, i, 0, buffer, COLS);
        int in_word = 0,is_line = 0;
        for (int j = 0; j < COLS; j++) 
        {
            if (isalnum(buffer[j])) 
            {
                if (!in_word) 
                {
                    num_words++;
                    
                    in_word = 1;
                    is_line = 1;
                }
                num_characters++;
            } 
            else 
            {
            
                in_word = 0;
            }
            
        }
        if(is_line)
            num_lines++;
     }
    printf("Number of lines: %d\n", num_lines);
    printf("Number of words: %d\n", num_words);
    printf("Number of characters: %d\n", num_characters);
    exit(0);
}

void handle_input(int ch) 
{
    switch (ch) 
    {
        case KEY_UP:
            if (cursor_y > 0) cursor_y--;
            break;
        case KEY_DOWN:
            if (cursor_y < LINES - 1) cursor_y++;
            break;
        case KEY_LEFT:
            if (cursor_x > 0) cursor_x--;
            break;
        case KEY_RIGHT:
            if (cursor_x < COLS - 1) cursor_x++;
            break;
        case 27: // Escape key
            handle_exit();
            break;
        case KEY_DC: // Delete key
        case KEY_BACKSPACE:
        case 127:
	   if(cursor_x > 0)
	   {
	   	mvwaddch(win, cursor_y, cursor_x,' ');
                cursor_x--;
                is_modified = 1;
                
	   }
	   else if (cursor_y > 0)
	   {
	   	mvwaddch(win, cursor_y, cursor_x,' ');
                cursor_y--;
                is_modified = 1;
                
	   }
	   break;
	

        case 19: // Ctrl + S
            save_file();
            break;
        case 24: // Ctrl + X
            save_file();
            handle_exit();
            break;
        default:
            if (isprint(ch)) 
            {
                mvwaddch(win, cursor_y, cursor_x, ch);
                cursor_x++;
                is_modified = 1;
                
            }
            break;
    }
  
}

static int my_func_backslash(int count, int key)
{
        printf("\\\n>");
        
        return 0;
}
static int my_func_edit(int count, int key)
{
	rl_point = 0;
	return 0;
}
static char *line_read = (char *)NULL;
char* rl_gets ()
{
  /* If the buffer has already been allocated, return the memory
     to the free pool. */
 rl_bind_key('\\',my_func_backslash);
 

 rl_bind_key(65,my_func_edit);//CTrl+a scii value =65
  if (line_read)
    {
      free (line_read);
      line_read = (char *)NULL;
    }

  /* Get a line from the user. */
  line_read = readline ("shell>");
  
 

  /* If the line has any text in it, save it on the history. */
  if (line_read && *line_read)
    add_history (line_read);

  return (line_read);
}

struct vector_args
{
	int *arr1;
	int *arr2;
	int *ans;
	int first;
	int last;
	int count;
};

void* sub_func(void *arg)
{
	struct vector_args *vec = (struct vector_args *)arg;
	int i = vec->first, j = vec->last , all = vec->count;

	for(i; i < j && i < all ; i++)
		vec->ans[i] = vec->arr1[i] - vec->arr2[i];

	return NULL;
}
void* dot_func(void *arg)
{
	struct vector_args *vec = (struct vector_args *)arg;
	int i = vec->first, j = vec->last , all = vec->count;

	for(i; i < j && i < all ; i++)
		vec->ans[i] = vec->arr1[i] * vec->arr2[i];

	return NULL;
}

void* add_func(void *arg)
{
	struct vector_args *vec = (struct vector_args *)arg;
	int i = vec->first, j = vec->last , all = vec->count;

	for(i; i < j && i < all ; i++)
		vec->ans[i] = vec->arr1[i] + vec->arr2[i];

	return NULL;
}

void function_addvec(char *f1, char *f2, int threadno)
{
    pthread_t threads_add[threadno];

    int no_values = 0, d;
    FILE *ptr = fopen(f1, "r");
    if (ptr == NULL)
    {
        printf("no such file1.");
        return;
    }
    while (fscanf(ptr, "%d ", &d) != -1)
    {
        no_values++;
    }
    fclose(ptr);

    FILE *file1 = fopen(f1, "r");
    FILE *file2 = fopen(f2, "r");
    if (file1 == NULL)
    {
        printf("no such file1.");
        return;
    }
    if (file2 == NULL)
    {
        printf("no such file2.");
        return;
    }

    int *v1 = (int *)malloc(no_values * sizeof(int));
    int *v2 = (int *)malloc(no_values * sizeof(int));
    
    for (int i = 0; i < no_values; i++)
    {
        fscanf(file1, "%d ", &v1[i]);
    }
    for (int i = 0; i < no_values; i++)
    {
        fscanf(file2, "%d ", &v2[i]);
    }

    int size = no_values / threadno;
    if (no_values % threadno != 0)
    {
        size++;
    }
    struct vector_args *vec[threadno];
    for (int i = 0; i < threadno; i++)
    {
        vec[i] = (struct vector_args *)malloc(sizeof(struct vector_args));

        vec[i]->arr1 = (int *)malloc(no_values * sizeof(int));
        vec[i]->arr2 = (int *)malloc(no_values * sizeof(int));
        vec[i]->ans = (int *)malloc(no_values * sizeof(int));
        vec[i]->count = no_values;

        for (int j = 0; j < no_values; j++) // Use j for the inner loops
        {
            vec[i]->arr1[j] = v1[j];
            vec[i]->arr2[j] = v2[j];
        }

        
        vec[i]->first = i * size;
        vec[i]->last = (i == threadno - 1) ? no_values : (i + 1) * size;
        pthread_create(&threads_add[i], NULL, add_func, vec[i]);
    }

    for (int i = 0; i < threadno; i++)
    {
        pthread_join(threads_add[i], NULL);
    }

    for (int i = 0; i < threadno; i++)
    {
        int first = i * size;
        int second = (i == threadno - 1) ? no_values : (i + 1) * size;
        for (int j = first; j < second; j++)
        {
            printf("%d ", vec[i]->ans[j]);
        }
    }
}

void function_subvec(char *f1, char *f2, int threadno)
{
    pthread_t threads_sub[threadno];

    int no_values = 0, d;
    FILE *ptr = fopen(f1, "r");
    if (ptr == NULL)
    {
        printf("no such file1.");
        return;
    }
    while (fscanf(ptr, "%d ", &d) != -1)
    {
        no_values++;
    }
    fclose(ptr);

    FILE *file1 = fopen(f1, "r");
    FILE *file2 = fopen(f2, "r");
    if (file1 == NULL)
    {
        printf("no such file1.");
        return;
    }
    if (file2 == NULL)
    {
        printf("no such file2.");
        return;
    }

    int *v1 = (int *)malloc(no_values * sizeof(int));
    int *v2 = (int *)malloc(no_values * sizeof(int));
    
    for (int i = 0; i < no_values; i++)
    {
        fscanf(file1, "%d ", &v1[i]);
    }
    for (int i = 0; i < no_values; i++)
    {
        fscanf(file2, "%d ", &v2[i]);
    }

    int size = no_values / threadno;
    if (no_values % threadno != 0)
    {
        size++;
    }
    struct vector_args *vec[threadno];
    for (int i = 0; i < threadno; i++)
    {
        vec[i] = (struct vector_args *)malloc(sizeof(struct vector_args));

        vec[i]->arr1 = (int *)malloc(no_values * sizeof(int));
        vec[i]->arr2 = (int *)malloc(no_values * sizeof(int));
        vec[i]->ans = (int *)malloc(no_values * sizeof(int));
        vec[i]->count = no_values;

        for (int j = 0; j < no_values; j++) // Use j for the inner loops
        {
            vec[i]->arr1[j] = v1[j];
            vec[i]->arr2[j] = v2[j];
        }

        
        vec[i]->first = i * size;
        vec[i]->last = (i == threadno - 1) ? no_values : (i + 1) * size;
        pthread_create(&threads_sub[i], NULL, sub_func, vec[i]);
    }

    for (int i = 0; i < threadno; i++)
    {
        pthread_join(threads_sub[i], NULL);
    }

    for (int i = 0; i < threadno; i++)
    {
        int first = i * size;
        int second = (i == threadno - 1) ? no_values : (i + 1) * size;
        for (int j = first; j < second; j++)
        {
            printf("%d ", vec[i]->ans[j]);
        }
    }
}

void function_dotvec(char *f1, char *f2, int threadno)
{
    pthread_t threads_dot[threadno];

    int no_values = 0, d;
    FILE *ptr = fopen(f1, "r");
    if (ptr == NULL)
    {
        printf("no such file1.");
        return;
    }
    while (fscanf(ptr, "%d ", &d) != -1)
    {
        no_values++;
    }
    fclose(ptr);

    FILE *file1 = fopen(f1, "r");
    FILE *file2 = fopen(f2, "r");
    if (file1 == NULL)
    {
        printf("no such file1.");
        return;
    }
    if (file2 == NULL)
    {
        printf("no such file2.");
        return;
    }

    int *v1 = (int *)malloc(no_values * sizeof(int));
    int *v2 = (int *)malloc(no_values * sizeof(int));
    
    for (int i = 0; i < no_values; i++)
    {
        fscanf(file1, "%d ", &v1[i]);
    }
    for (int i = 0; i < no_values; i++)
    {
        fscanf(file2, "%d ", &v2[i]);
    }

    int size = no_values / threadno;
    if (no_values % threadno != 0)
    {
        size++;
    }
    struct vector_args *vec[threadno];
    for (int i = 0; i < threadno; i++)
    {
        vec[i] = (struct vector_args *)malloc(sizeof(struct vector_args));

        vec[i]->arr1 = (int *)malloc(no_values * sizeof(int));
        vec[i]->arr2 = (int *)malloc(no_values * sizeof(int));
        vec[i]->ans = (int *)malloc(no_values * sizeof(int));
        vec[i]->count = no_values;

        for (int j = 0; j < no_values; j++) // Use j for the inner loops
        {
            vec[i]->arr1[j] = v1[j];
            vec[i]->arr2[j] = v2[j];
        }

        
        vec[i]->first = i * size;
        vec[i]->last = (i == threadno - 1) ? no_values : (i + 1) * size;
        pthread_create(&threads_dot[i], NULL, dot_func, vec[i]);
    }

    for (int i = 0; i < threadno; i++)
    {
        pthread_join(threads_dot[i], NULL);
    }

    for (int i = 0; i < threadno; i++)
    {
        int first = i * size;
        int second = (i == threadno - 1) ? no_values : (i + 1) * size;
        for (int j = first; j < second; j++)
        {
            printf("%d ", vec[i]->ans[j]);
        }
    }
}


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

void call_create_vi_terminal()
{
    int fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening file");
        exit(1);
    }
    close(fd);
    signal(SIGINT, handle_exit);

    initscr();
    raw();
    cbreak();
    noecho();
    curs_set(2);

    win = newwin(LINES, COLS, 0, 0);
    keypad(win, TRUE);  // Enable keypad mode for arrow keys

    // Check if the file exists and read its content
    load_file();

    

    // Initialize some values
    cursor_x = cursor_y = 0;

    while (1) 
    {
        wmove(win, cursor_y, cursor_x);
        wrefresh(win);
        int ch = wgetch(win);
        handle_input(ch);
    }
}

int main() {
    while (1) {
        char *command =rl_gets();
        if(strcmp(command,"exit") == 0)
            break;

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
                 printf("pwd\ncd <dir name>\nmkdir <dir name>\nls <flag>\nexit\nhelp\nvi\naddvec\nsubvec\ndotprod\n");
                 continue;
            }
            else if (strcmp(token_all[0],"vi") == 0)
            {
            	if(token_all[1] != NULL)
            	{
            		pid_t child_vi = fork();
            		if(child_vi == -1)
            		{
            			perror("child vi failed");
                                exit(1);
            		}
            		if(child_vi == 0 )
            		{
            			filename = token_all[1];
            			call_create_vi_terminal();
            		}
            		else
            		{
            			waitpid(child_vi,NULL,0);
            		}
            	}
            	else
            	{
            		perror("Usage: vi <filename>/n");
            		exit(1);
            	}
            	continue;
            }
            else if(strcmp(token_all[0],"addvec") == 0)
	        {
	        	//printf("%s",command);
	        	if(no_tokens == 4)
	        	{
	        		int threadno = atoi(token_all[3]);
	        		function_addvec(token_all[1],token_all[2],threadno);
	        	}
	        	else
	        	{
	        		perror("Wrong usage");
	        		exit(1);
	        	}
	        	continue;
	        }
	        else if (strcmp(token_all[0], "subvec") == 0)
        	{
	        	//function_subvec(command);
	        	if(no_tokens == 4)
	        	{
	        		int threadno = atoi(token_all[3]);
	        		function_subvec(token_all[1],token_all[2],threadno);
	        	}
	        	else
	        	{
	        		perror("Wrong usage");
	        		exit(1);
	        	}
	        	continue;
	        }
	        else if(strcmp(token_all[0],"dotprod") == 0)
	        {
	        	//function_dotprod(command);
	        	if(no_tokens == 4)
	        	{
	        		int threadno = atoi(token_all[3]);
	        		function_dotvec(token_all[1],token_all[2],threadno);
	        	}
	        	else
	        	{
	        		perror("Wrong usage");
	        		exit(1);
	        	}
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

