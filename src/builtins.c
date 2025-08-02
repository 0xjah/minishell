#include "../include/minishell.h"

int	is_builtin(char *cmd)
{
	if (!cmd)
		return (0);
	
	if (strcmp(cmd, "echo") == 0 ||
		strcmp(cmd, "cd") == 0 ||
		strcmp(cmd, "pwd") == 0 ||
		strcmp(cmd, "export") == 0 ||
		strcmp(cmd, "unset") == 0 ||
		strcmp(cmd, "env") == 0 ||
		strcmp(cmd, "exit") == 0)
		return (1);
	
	return (0);
}

int	execute_builtin(t_cmd *cmd)
{
	if (!cmd || !cmd->args || !cmd->args[0])
		return (1);
	
	if (strcmp(cmd->args[0], "echo") == 0)
		return (builtin_echo(cmd->args));
	else if (strcmp(cmd->args[0], "cd") == 0)
		return (builtin_cd(cmd->args));
	else if (strcmp(cmd->args[0], "pwd") == 0)
		return (builtin_pwd(cmd->args));
	else if (strcmp(cmd->args[0], "export") == 0)
		return (builtin_export(cmd->args));
	else if (strcmp(cmd->args[0], "unset") == 0)
		return (builtin_unset(cmd->args));
	else if (strcmp(cmd->args[0], "env") == 0)
		return (builtin_env(cmd->args));
	else if (strcmp(cmd->args[0], "exit") == 0)
		return (builtin_exit(cmd->args));
	
	return (1);
}

int	builtin_echo(char **args)
{
	int	i;
	int	newline;

	newline = 1;
	i = 1;
	
	/* Check for -n flag */
	if (args[i] && strcmp(args[i], "-n") == 0)
	{
		newline = 0;
		i++;
	}
	
	/* Print arguments */
	while (args[i])
	{
		printf("%s", args[i]);
		if (args[i + 1])
			printf(" ");
		i++;
	}
	
	if (newline)
		printf("\n");
	
	return (0);
}

int	builtin_cd(char **args)
{
	char	*path;
	char	*home;
	char	*oldpwd;
	char	cwd[MAX_PATH];

	/* Get current directory for OLDPWD */
	if (getcwd(cwd, MAX_PATH))
		oldpwd = cwd;
	else
		oldpwd = get_env_value("PWD");
	
	/* Determine target directory */
	if (!args[1] || strcmp(args[1], "~") == 0)
	{
		home = get_env_value("HOME");
		if (!home)
		{
			print_error("cd", "HOME not set");
			return (1);
		}
		path = home;
	}
	else if (strcmp(args[1], "-") == 0)
	{
		path = get_env_value("OLDPWD");
		if (!path)
		{
			print_error("cd", "OLDPWD not set");
			return (1);
		}
		printf("%s\n", path);
	}
	else
		path = args[1];
	
	/* Change directory */
	if (chdir(path) != 0)
	{
		print_error("cd", strerror(errno));
		return (1);
	}
	
	/* Update environment variables */
	set_env_value("OLDPWD", oldpwd);
	if (getcwd(cwd, MAX_PATH))
		set_env_value("PWD", cwd);
	
	return (0);
}

int	builtin_pwd(char **args)
{
	char	cwd[MAX_PATH];

	(void)args;
	
	if (getcwd(cwd, MAX_PATH))
	{
		printf("%s\n", cwd);
		return (0);
	}
	else
	{
		print_error("pwd", strerror(errno));
		return (1);
	}
}

int	builtin_export(char **args)
{
	char	*key;
	char	*value;
	char	*equals;
	int		i;

	if (!args[1])
	{
		/* Print all exported variables */
		builtin_env(args);
		return (0);
	}
	
	i = 1;
	while (args[i])
	{
		equals = strchr(args[i], '=');
		if (equals)
		{
			*equals = '\0';
			key = args[i];
			value = equals + 1;
			set_env_value(key, value);
			*equals = '=';  // Restore original string
		}
		else
		{
			/* Export variable without value */
			set_env_value(args[i], "");
		}
		i++;
	}
	
	return (0);
}

int	builtin_unset(char **args)
{
	int	i;

	if (!args[1])
		return (0);
	
	i = 1;
	while (args[i])
	{
		unset_env_value(args[i]);
		i++;
	}
	
	return (0);
}

int	builtin_env(char **args)
{
	t_env	*current;

	(void)args;
	
	current = g_shell.env_list;
	while (current)
	{
		if (current->value && *current->value)
			printf("%s=%s\n", current->key, current->value);
		current = current->next;
	}
	
	return (0);
}

int	builtin_exit(char **args)
{
	int	exit_code;

	exit_code = g_shell.exit_status;
	
	if (args[1])
	{
		exit_code = atoi(args[1]);
		if (args[2])
		{
			print_error("exit", "too many arguments");
			return (1);
		}
	}
	
	printf("exit\n");
	cleanup_shell();
	exit(exit_code);
}
