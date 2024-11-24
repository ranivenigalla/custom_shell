#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include<string.h>

int  COMMAND_SIZE = 5000;
int WD_SIZE = 10000;
void execute_comand(char **token_all, int flag)
{

	pid_t child_pid = fork();

	if(child_pid == -1)
	{
		perror("Fork failed:");
		exit(1);
	}
	
	if(child_pid == 0)
	{	
		if(execvp(token_all[0],token_all) == -1)
		{
			perror("exec failed");
			exit(1);
		}
	}
	else
	{
		if(flag)
		{
			waitpid(child_pid,NULL,0);
		}
	}

}


int main()
{
	while(1)
	{
		printf("shell>");

		char command[COMMAND_SIZE];

		scanf("%4999[^\n]%*c",command);	
		;//Reading the command by omitting /n
		if(strcmp(command,"exit") == 0)
		{
			exit(0);

		}
		//printf("%s ",command);
		char *token_all[COMMAND_SIZE];

		char *token = NULL;
		
		int no_tokens = 0;
		token = strtok(command," ");
		
		token_all[no_tokens++] = token;

		while((token = strtok(NULL," "))!=NULL)
		{
			token_all[no_tokens++] =  token;
			//printf("%s\n",token);

		}
		//printf("%d ",no_tokens);
		int flag = 1;
		int len_and = strlen(token_all[no_tokens - 1]);
		//printf("%d ",len_and);
		if(token_all[no_tokens - 1][len_and - 1] == '&')
		{
			flag = 0;
		}


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
			execute_comand(token_all,flag);
		}
		
	}
	return 0;
}