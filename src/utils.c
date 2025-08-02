#include "../include/minishell.h"

void	*safe_malloc(size_t size)
{
	void	*ptr;

	ptr = malloc(size);
	if (!ptr)
		exit_error("malloc failed");
	return (ptr);
}

char	*safe_strdup(char *str)
{
	char	*dup;
	int		len;
	int		i;

	if (!str)
		return (NULL);
	
	len = strlen(str);
	dup = safe_malloc(len + 1);
	
	i = 0;
	while (i < len)
	{
		dup[i] = str[i];
		i++;
	}
	dup[i] = '\0';
	
	return (dup);
}

char	*join_strings(char *s1, char *s2)
{
	char	*result;
	int		len1;
	int		len2;
	int		i;
	int		j;

	if (!s1 || !s2)
		return (NULL);
	
	len1 = strlen(s1);
	len2 = strlen(s2);
	result = safe_malloc(len1 + len2 + 1);
	
	i = 0;
	while (i < len1)
	{
		result[i] = s1[i];
		i++;
	}
	
	j = 0;
	while (j < len2)
	{
		result[i + j] = s2[j];
		j++;
	}
	
	result[i + j] = '\0';
	return (result);
}

int	count_words(char *str, char delimiter)
{
	int	count;
	int	in_word;
	int	i;

	if (!str)
		return (0);
	
	count = 0;
	in_word = 0;
	i = 0;
	
	while (str[i])
	{
		if (str[i] != delimiter && !in_word)
		{
			in_word = 1;
			count++;
		}
		else if (str[i] == delimiter)
			in_word = 0;
		i++;
	}
	
	return (count);
}

char	**split_string(char *str, char delimiter)
{
	char	**result;
	int		word_count;
	int		i;
	int		j;
	int		start;

	if (!str)
		return (NULL);
	
	word_count = count_words(str, delimiter);
	result = safe_malloc(sizeof(char *) * (word_count + 1));
	
	i = 0;
	j = 0;
	while (str[i] && j < word_count)
	{
		/* Skip delimiters */
		while (str[i] && str[i] == delimiter)
			i++;
		
		start = i;
		/* Find end of word */
		while (str[i] && str[i] != delimiter)
			i++;
		
		if (i > start)
		{
			result[j] = safe_malloc(i - start + 1);
			strncpy(result[j], str + start, i - start);
			result[j][i - start] = '\0';
			j++;
		}
	}
	
	result[j] = NULL;
	return (result);
}

char	*trim_whitespace(char *str)
{
	char	*start;
	char	*end;
	char	*result;
	int		len;

	if (!str)
		return (NULL);
	
	/* Find start */
	start = str;
	while (*start && strchr(" \t\n", *start))
		start++;
	
	/* Find end */
	end = start + strlen(start) - 1;
	while (end > start && strchr(" \t\n", *end))
		end--;
	
	/* Calculate length and create result */
	len = end - start + 1;
	result = safe_malloc(len + 1);
	strncpy(result, start, len);
	result[len] = '\0';
	
	return (result);
}

void	free_string_array(char **array)
{
	int	i;

	if (!array)
		return ;
	
	i = 0;
	while (array[i])
	{
		free(array[i]);
		i++;
	}
	free(array);
}

int	array_length(char **array)
{
	int	len;

	if (!array)
		return (0);
	
	len = 0;
	while (array[len])
		len++;
	
	return (len);
}

void	print_error(char *cmd, char *msg)
{
	write(STDERR_FILENO, "minishell: ", 11);
	if (cmd)
	{
		write(STDERR_FILENO, cmd, strlen(cmd));
		write(STDERR_FILENO, ": ", 2);
	}
	write(STDERR_FILENO, msg, strlen(msg));
	write(STDERR_FILENO, "\n", 1);
}

void	exit_error(char *msg)
{
	print_error(NULL, msg);
	exit(1);
}

int	syntax_error(char *token)
{
	write(STDERR_FILENO, "minishell: syntax error near unexpected token `", 47);
	if (token)
		write(STDERR_FILENO, token, strlen(token));
	else
		write(STDERR_FILENO, "newline", 7);
	write(STDERR_FILENO, "'\n", 2);
	return (0);
}

void	cleanup_shell(void)
{
	free_env();
	if (g_shell.pids)
		free(g_shell.pids);
	close(g_shell.stdin_backup);
	close(g_shell.stdout_backup);
}
