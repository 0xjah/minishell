#include "../include/minishell.h"

t_token	*create_token(t_token_type type, char *value)
{
	t_token	*token;

	token = safe_malloc(sizeof(t_token));
	token->type = type;
	token->value = value ? safe_strdup(value) : NULL;
	token->next = NULL;
	return (token);
}

void	add_token(t_token **tokens, t_token *new_token)
{
	t_token	*current;

	if (!*tokens)
	{
		*tokens = new_token;
		return ;
	}
	current = *tokens;
	while (current->next)
		current = current->next;
	current->next = new_token;
}

void	free_tokens(t_token *tokens)
{
	t_token	*next;

	while (tokens)
	{
		next = tokens->next;
		if (tokens->value)
			free(tokens->value);
		free(tokens);
		tokens = next;
	}
}

char	*extract_word(char *input, int *i)
{
	int		start;
	int		len;
	char	*word;

	start = *i;
	while (input[*i] && !strchr(" \t\n|<>&();", input[*i]))
		(*i)++;
	len = *i - start;
	word = safe_malloc(len + 1);
	strncpy(word, input + start, len);
	word[len] = '\0';
	return (word);
}

char	*extract_quoted_string(char *input, int *i, char quote)
{
	int		start;
	int		len;
	char	*str;

	start = ++(*i);  // Skip opening quote
	while (input[*i] && input[*i] != quote)
		(*i)++;
	if (input[*i] != quote)
	{
		print_error("lexer", "unterminated quoted string");
		return (NULL);
	}
	len = *i - start;
	str = safe_malloc(len + 1);
	strncpy(str, input + start, len);
	str[len] = '\0';
	(*i)++;  // Skip closing quote
	return (str);
}

static t_token_type	get_operator_type(char *input, int *i)
{
	if (input[*i] == '|')
	{
		if (input[*i + 1] == '|')
		{
			*i += 2;
			return (TOKEN_OR);
		}
		(*i)++;
		return (TOKEN_PIPE);
	}
	else if (input[*i] == '<')
	{
		if (input[*i + 1] == '<')
		{
			*i += 2;
			return (TOKEN_REDIRECT_HEREDOC);
		}
		(*i)++;
		return (TOKEN_REDIRECT_IN);
	}
	else if (input[*i] == '>')
	{
		if (input[*i + 1] == '>')
		{
			*i += 2;
			return (TOKEN_REDIRECT_APPEND);
		}
		(*i)++;
		return (TOKEN_REDIRECT_OUT);
	}
	else if (input[*i] == '&')
	{
		if (input[*i + 1] == '&')
		{
			*i += 2;
			return (TOKEN_AND);
		}
	}
	else if (input[*i] == '(')
	{
		(*i)++;
		return (TOKEN_LPAREN);
	}
	else if (input[*i] == ')')
	{
		(*i)++;
		return (TOKEN_RPAREN);
	}
	return (TOKEN_ERROR);
}

t_token	*lexer(char *input)
{
	t_token	*tokens;
	t_token	*new_token;
	char	*word;
	int		i;

	tokens = NULL;
	i = 0;
	
	while (input[i])
	{
		/* Skip whitespace */
		while (input[i] && strchr(" \t\n", input[i]))
			i++;
		
		if (!input[i])
			break ;
		
		/* Handle quoted strings */
		if (input[i] == '"' || input[i] == '\'')
		{
			word = extract_quoted_string(input, &i, input[i]);
			if (!word)
			{
				free_tokens(tokens);
				return (NULL);
			}
			new_token = create_token(TOKEN_WORD, word);
			free(word);
		}
		/* Handle operators */
		else if (strchr("|<>&();", input[i]))
		{
			t_token_type type = get_operator_type(input, &i);
			if (type == TOKEN_ERROR)
			{
				free_tokens(tokens);
				return (NULL);
			}
			new_token = create_token(type, NULL);
		}
		/* Handle regular words */
		else
		{
			word = extract_word(input, &i);
			new_token = create_token(TOKEN_WORD, word);
			free(word);
		}
		
		add_token(&tokens, new_token);
	}
	
	/* Add EOF token */
	add_token(&tokens, create_token(TOKEN_EOF, NULL));
	return (tokens);
}
