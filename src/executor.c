#include "../include/minishell.h"

char	*find_command_path(char *cmd)
{
	char	*path_env;
	char	**paths;
	char	*full_path;
	char	*temp;
	int		i;

	if (!cmd || !*cmd)
		return (NULL);
	
	/* If command contains '/', it's a path */
	if (strchr(cmd, '/'))
	{
		if (access(cmd, X_OK) == 0)
			return (safe_strdup(cmd));
		return (NULL);
	}
	
	path_env = get_env_value("PATH");
	if (!path_env)
		return (NULL);
	
	paths = split_string(path_env, ':');
	if (!paths)
		return (NULL);
	
	i = 0;
	while (paths[i])
	{
		temp = join_strings(paths[i], "/");
		full_path = join_strings(temp, cmd);
		free(temp);
		
		if (access(full_path, X_OK) == 0)
		{
			free_string_array(paths);
			return (full_path);
		}
		free(full_path);
		i++;
	}
	
	free_string_array(paths);
	return (NULL);
}

static int	setup_redirections(t_cmd *cmd)
{
	int	fd;

	if (cmd->input_file)
	{
		fd = open(cmd->input_file, O_RDONLY);
		if (fd == -1)
		{
			print_error(cmd->input_file, strerror(errno));
			return (0);
		}
		dup2(fd, STDIN_FILENO);
		close(fd);
	}
	
	if (cmd->heredoc)
	{
		fd = create_heredoc(cmd->heredoc_delimiter);
		if (fd == -1)
			return (0);
		dup2(fd, STDIN_FILENO);
		close(fd);
	}
	
	if (cmd->output_file)
	{
		if (cmd->append_output)
			fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
		else
			fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		
		if (fd == -1)
		{
			print_error(cmd->output_file, strerror(errno));
			return (0);
		}
		dup2(fd, STDOUT_FILENO);
		close(fd);
	}
	
	return (1);
}

int	execute_single_cmd(t_cmd *cmd)
{
	pid_t	pid;
	int		status;
	char	*cmd_path;

	if (!cmd || !cmd->args || !cmd->args[0])
		return (0);
	
	/* Check if it's a builtin command */
	if (is_builtin(cmd->args[0]))
	{
		if (!setup_redirections(cmd))
			return (1);
		return (execute_builtin(cmd));
	}
	
	/* Find command path */
	cmd_path = find_command_path(cmd->args[0]);
	if (!cmd_path)
	{
		print_error(cmd->args[0], "command not found");
		return (127);
	}
	
	pid = fork();
	if (pid == 0)
	{
		/* Child process */
		if (!setup_redirections(cmd))
			exit(1);
		
		execve(cmd_path, cmd->args, g_shell.env_array);
		print_error(cmd->args[0], strerror(errno));
		exit(126);
	}
	else if (pid > 0)
	{
		/* Parent process */
		waitpid(pid, &status, 0);
		free(cmd_path);
		if (WIFEXITED(status))
			return (WEXITSTATUS(status));
		return (1);
	}
	else
	{
		print_error("fork", strerror(errno));
		free(cmd_path);
		return (1);
	}
}

int	execute_pipeline(t_cmd *cmds)
{
	int		pipe_fds[2];
	pid_t	pid;
	t_cmd	*current;
	int		prev_fd;
	int		status;
	int		last_exit_status;

	if (!cmds->next)
		return (execute_single_cmd(cmds));
	
	prev_fd = -1;
	current = cmds;
	last_exit_status = 0;
	
	while (current)
	{
		if (current->next && pipe(pipe_fds) == -1)
		{
			print_error("pipe", strerror(errno));
			return (1);
		}
		
		pid = fork();
		if (pid == 0)
		{
			/* Child process */
			if (prev_fd != -1)
			{
				dup2(prev_fd, STDIN_FILENO);
				close(prev_fd);
			}
			
			if (current->next)
			{
				dup2(pipe_fds[1], STDOUT_FILENO);
				close(pipe_fds[1]);
			}
			
			if (current->next)
				close(pipe_fds[0]);
			
			if (!setup_redirections(current))
				exit(1);
			
			if (is_builtin(current->args[0]))
			{
				exit(execute_builtin(current));
			}
			else
			{
				char *cmd_path = find_command_path(current->args[0]);
				if (!cmd_path)
				{
					print_error(current->args[0], "command not found");
					exit(127);
				}
				execve(cmd_path, current->args, g_shell.env_array);
				print_error(current->args[0], strerror(errno));
				exit(126);
			}
		}
		else if (pid > 0)
		{
			/* Parent process */
			if (prev_fd != -1)
				close(prev_fd);
			
			if (current->next)
			{
				close(pipe_fds[1]);
				prev_fd = pipe_fds[0];
			}
			
			if (!current->next)
			{
				waitpid(pid, &status, 0);
				if (WIFEXITED(status))
					last_exit_status = WEXITSTATUS(status);
			}
		}
		else
		{
			print_error("fork", strerror(errno));
			return (1);
		}
		
		current = current->next;
	}
	
	/* Wait for all remaining children */
	while (wait(NULL) > 0)
		;
	
	return (last_exit_status);
}

int	executor(t_cmd *cmds)
{
	int	exit_status;

	if (!cmds)
		return (0);
	
	/* Backup stdin/stdout */
	dup2(g_shell.stdin_backup, STDIN_FILENO);
	dup2(g_shell.stdout_backup, STDOUT_FILENO);
	
	exit_status = execute_pipeline(cmds);
	
	/* Restore stdin/stdout */
	dup2(g_shell.stdin_backup, STDIN_FILENO);
	dup2(g_shell.stdout_backup, STDOUT_FILENO);
	
	g_shell.exit_status = exit_status;
	return (exit_status);
}
