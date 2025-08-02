#include "../include/minishell.h"

t_cmd	*create_cmd(void)
{
	t_cmd	*cmd;

	cmd = safe_malloc(sizeof(t_cmd));
	cmd->args = NULL;
	cmd->input_file = NULL;
	cmd->output_file = NULL;
	cmd->append_output = 0;
	cmd->heredoc = 0;
	cmd->heredoc_delimiter = NULL;
	cmd->next = NULL;
	return (cmd);
}

void	add_cmd(t_cmd **cmds, t_cmd *new_cmd)
{
	t_cmd	*current;

	if (!*cmds)
	{
		*cmds = new_cmd;
		return ;
	}
	current = *cmds;
	while (current->next)
		current = current->next;
	current->next = new_cmd;
}

void	free_cmds(t_cmd *cmds)
{
	t_cmd	*next;
	int		i;

	while (cmds)
	{
		next = cmds->next;
		if (cmds->args)
		{
			i = 0;
			while (cmds->args[i])
				free(cmds->args[i++]);
			free(cmds->args);
		}
		if (cmds->input_file)
			free(cmds->input_file);
		if (cmds->output_file)
			free(cmds->output_file);
		if (cmds->heredoc_delimiter)
			free(cmds->heredoc_delimiter);
		free(cmds);
		cmds = next;
	}
}

static void	add_arg_to_cmd(t_cmd *cmd, char *arg)
{
	int		count;
	char	**new_args;
	char	*expanded_arg;
	int		i;

	/* Expand variables and tilde */
	expanded_arg = expand_variables(arg);
	if (expanded_arg)
	{
		char *tilde_expanded = expand_tilde(expanded_arg);
		free(expanded_arg);
		expanded_arg = tilde_expanded;
	}
	else
		expanded_arg = safe_strdup(arg);

	/* Count existing arguments */
	count = 0;
	if (cmd->args)
	{
		while (cmd->args[count])
			count++;
	}
	
	/* Allocate new array */
	new_args = safe_malloc(sizeof(char *) * (count + 2));
	
	/* Copy existing arguments */
	i = 0;
	if (cmd->args)
	{
		while (i < count)
		{
			new_args[i] = cmd->args[i];
			i++;
		}
		free(cmd->args);
	}
	
	/* Add new argument */
	new_args[count] = expanded_arg;
	new_args[count + 1] = NULL;
	cmd->args = new_args;
}

int	parse_redirections(t_token **tokens, t_cmd *cmd)
{
	t_token	*current;

	current = *tokens;
	
	if (current->type == TOKEN_REDIRECT_IN)
	{
		*tokens = current->next;
		if (!*tokens || (*tokens)->type != TOKEN_WORD)
			return (syntax_error("newline"));
		cmd->input_file = safe_strdup((*tokens)->value);
		*tokens = (*tokens)->next;
	}
	else if (current->type == TOKEN_REDIRECT_OUT)
	{
		*tokens = current->next;
		if (!*tokens || (*tokens)->type != TOKEN_WORD)
			return (syntax_error("newline"));
		cmd->output_file = safe_strdup((*tokens)->value);
		cmd->append_output = 0;
		*tokens = (*tokens)->next;
	}
	else if (current->type == TOKEN_REDIRECT_APPEND)
	{
		*tokens = current->next;
		if (!*tokens || (*tokens)->type != TOKEN_WORD)
			return (syntax_error("newline"));
		cmd->output_file = safe_strdup((*tokens)->value);
		cmd->append_output = 1;
		*tokens = (*tokens)->next;
	}
	else if (current->type == TOKEN_REDIRECT_HEREDOC)
	{
		*tokens = current->next;
		if (!*tokens || (*tokens)->type != TOKEN_WORD)
			return (syntax_error("newline"));
		cmd->heredoc = 1;
		cmd->heredoc_delimiter = safe_strdup((*tokens)->value);
		*tokens = (*tokens)->next;
	}
	return (1);
}

int	parse_command(t_token **tokens, t_cmd *cmd)
{
	while (*tokens && (*tokens)->type != TOKEN_PIPE && 
		   (*tokens)->type != TOKEN_EOF)
	{
		if ((*tokens)->type == TOKEN_WORD)
		{
			add_arg_to_cmd(cmd, (*tokens)->value);
			*tokens = (*tokens)->next;
		}
		else if ((*tokens)->type == TOKEN_REDIRECT_IN ||
				 (*tokens)->type == TOKEN_REDIRECT_OUT ||
				 (*tokens)->type == TOKEN_REDIRECT_APPEND ||
				 (*tokens)->type == TOKEN_REDIRECT_HEREDOC)
		{
			if (!parse_redirections(tokens, cmd))
				return (0);
		}
		else
		{
			return (syntax_error((*tokens)->value));
		}
	}
	return (1);
}

t_cmd	*parser(t_token *tokens)
{
	t_cmd	*commands;
	t_cmd	*current_cmd;

	commands = NULL;
	
	while (tokens && tokens->type != TOKEN_EOF)
	{
		current_cmd = create_cmd();
		
		if (!parse_command(&tokens, current_cmd))
		{
			free_cmds(commands);
			free_cmds(current_cmd);
			return (NULL);
		}
		
		add_cmd(&commands, current_cmd);
		
		/* Handle pipe */
		if (tokens && tokens->type == TOKEN_PIPE)
		{
			tokens = tokens->next;
			if (!tokens || tokens->type == TOKEN_EOF)
			{
				syntax_error("newline");
				free_cmds(commands);
				return (NULL);
			}
		}
	}
	
	return (commands);
}
