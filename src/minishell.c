#include "../include/minishell.h"

t_shell	g_shell;

static void	init_shell(char **envp)
{
	g_shell.exit_status = 0;
	g_shell.stdin_backup = dup(STDIN_FILENO);
	g_shell.stdout_backup = dup(STDOUT_FILENO);
	g_shell.pids = NULL;
	g_shell.num_processes = 0;
	init_env(envp);
	setup_signals();
}

static int	process_input(char *input)
{
	t_token	*tokens;
	t_cmd	*commands;
	int		result;

	if (!input || !*input)
		return (0);
	
	/* Add to history */
	add_history(input);
	
	/* Lexical analysis */
	tokens = lexer(input);
	if (!tokens)
		return (0);
	
	/* Syntax analysis */
	commands = parser(tokens);
	free_tokens(tokens);
	if (!commands)
		return (0);
	
	/* Execution */
	result = executor(commands);
	free_cmds(commands);
	
	return (result);
}

static void	shell_loop(void)
{
	char	*input;
	char	*prompt;
	char	cwd[MAX_PATH];
	char	*home;
	char	*display_path;

	while (1)
	{
		/* Create magical prompt with ~ for home directory */
		if (getcwd(cwd, MAX_PATH))
		{
			home = get_env_value("HOME");
			if (home && strncmp(cwd, home, strlen(home)) == 0)
			{
				/* Replace home path with ~ */
				if (cwd[strlen(home)] == '\0')
					display_path = safe_strdup("~");
				else
				{
					display_path = malloc(strlen(cwd) - strlen(home) + 2);
					if (display_path)
						sprintf(display_path, "~%s", cwd + strlen(home));
				}
			}
			else
			{
				display_path = safe_strdup(cwd);
			}
			
			prompt = malloc(strlen(display_path) + 20);
			if (prompt)
				sprintf(prompt, "✨ %s\n🌟 ", display_path);
			
			if (display_path)
				free(display_path);
		}
		else
		{
			prompt = safe_strdup("✨ frieren_shell\n🌟 ");
		}
		
		input = readline(prompt ? prompt : "✨ frieren_shell\n🌟 ");
		
		if (prompt)
			free(prompt);
		
		/* Handle EOF (Ctrl+D) */
		if (!input)
		{
			printf("exit\n");
			break ;
		}
		
		/* Process the input */
		if (*input)
			process_input(input);
		
		free(input);
	}
}

int	main(int argc, char **argv, char **envp)
{
	(void)argc;
	(void)argv;
	
	init_shell(envp);
	
	printf("🌟 Welcome to Frieren's Magic Shell! 🧙‍♀️⚡\n");
	printf("Cast your spells or type 'exit' to return to the real world.\n");
	printf("May your journey be filled with magic! ✨\n\n");
	
	shell_loop();
	
	cleanup_shell();
	return (g_shell.exit_status);
}