#include "../include/minishell.h"

int	check_file_access(char *filename, int mode)
{
	if (access(filename, mode) == 0)
		return (1);
	return (0);
}

int	create_heredoc(char *delimiter)
{
	int		pipe_fds[2];
	char	*line;
	pid_t	pid;

	if (pipe(pipe_fds) == -1)
	{
		print_error("pipe", strerror(errno));
		return (-1);
	}
	
	pid = fork();
	if (pid == 0)
	{
		/* Child process - read input and write to pipe */
		close(pipe_fds[0]);
		
		while (1)
		{
			line = readline("> ");
			if (!line)
			{
				printf("\n");
				break ;
			}
			
			if (strcmp(line, delimiter) == 0)
			{
				free(line);
				break ;
			}
			
			write(pipe_fds[1], line, strlen(line));
			write(pipe_fds[1], "\n", 1);
			free(line);
		}
		
		close(pipe_fds[1]);
		exit(0);
	}
	else if (pid > 0)
	{
		/* Parent process */
		int status;
		close(pipe_fds[1]);
		waitpid(pid, &status, 0);
		return (pipe_fds[0]);
	}
	else
	{
		print_error("fork", strerror(errno));
		close(pipe_fds[0]);
		close(pipe_fds[1]);
		return (-1);
	}
}
