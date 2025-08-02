#include "../include/minishell.h"

void	handle_sigint(int sig)
{
	(void)sig;
	
	/* Print newline and redisplay prompt */
	printf("\n");
	rl_on_new_line();
	rl_replace_line("", 0);
	rl_redisplay();
	
	/* Set exit status */
	g_shell.exit_status = 130;
}

void	handle_sigquit(int sig)
{
	(void)sig;
	/* Do nothing - ignore SIGQUIT in interactive mode */
}

void	setup_signals(void)
{
	struct sigaction	sa_int;
	struct sigaction	sa_quit;

	/* Handle SIGINT (Ctrl+C) */
	sa_int.sa_handler = handle_sigint;
	sigemptyset(&sa_int.sa_mask);
	sa_int.sa_flags = SA_RESTART;
	sigaction(SIGINT, &sa_int, NULL);
	
	/* Ignore SIGQUIT (Ctrl+\) */
	sa_quit.sa_handler = handle_sigquit;
	sigemptyset(&sa_quit.sa_mask);
	sa_quit.sa_flags = SA_RESTART;
	sigaction(SIGQUIT, &sa_quit, NULL);
}
