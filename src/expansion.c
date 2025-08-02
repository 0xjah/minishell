#include "../include/minishell.h"

static char	*extract_var_name(char *str, int *i)
{
	int		start;
	int		len;
	char	*name;

	start = *i;
	
	/* Handle special variables */
	if (str[*i] == '?')
	{
		(*i)++;
		return (safe_strdup("?"));
	}
	
	/* Handle regular variable names */
	while (str[*i] && (isalnum(str[*i]) || str[*i] == '_'))
		(*i)++;
	
	len = *i - start;
	if (len == 0)
		return (NULL);
	
	name = safe_malloc(len + 1);
	strncpy(name, str + start, len);
	name[len] = '\0';
	
	return (name);
}

char	*expand_variables(char *str)
{
	char	*result;
	char	*var_name;
	char	*var_value;
	char	*temp;
	int		i;
	int		result_len;

	if (!str)
		return (NULL);
	
	result = safe_malloc(strlen(str) * 2 + 1);  // Conservative allocation
	result[0] = '\0';
	result_len = 0;
	
	i = 0;
	while (str[i])
	{
		if (str[i] == '$' && str[i + 1])
		{
			i++;  // Skip $
			var_name = extract_var_name(str, &i);
			if (var_name)
			{
				if (strcmp(var_name, "?") == 0)
				{
					var_value = safe_malloc(12);  // Enough for any exit status
					sprintf(var_value, "%d", g_shell.exit_status);
				}
				else
				{
					var_value = get_env_value(var_name);
					if (var_value)
						var_value = safe_strdup(var_value);
					else
						var_value = safe_strdup("");
				}
				
				/* Expand result buffer if needed */
				temp = safe_malloc(result_len + strlen(var_value) + 1);
				strcpy(temp, result);
				strcat(temp, var_value);
				free(result);
				result = temp;
				result_len += strlen(var_value);
				
				free(var_name);
				free(var_value);
			}
			else
			{
				/* Just add the $ character */
				temp = safe_malloc(result_len + 2);
				strcpy(temp, result);
				temp[result_len] = '$';
				temp[result_len + 1] = '\0';
				free(result);
				result = temp;
				result_len++;
			}
		}
		else
		{
			/* Regular character */
			temp = safe_malloc(result_len + 2);
			strcpy(temp, result);
			temp[result_len] = str[i];
			temp[result_len + 1] = '\0';
			free(result);
			result = temp;
			result_len++;
			i++;
		}
	}
	
	return (result);
}

char	*expand_tilde(char *str)
{
	char	*home;
	char	*result;

	if (!str || str[0] != '~')
		return (safe_strdup(str));
	
	home = get_env_value("HOME");
	if (!home)
		return (safe_strdup(str));
	
	if (str[1] == '\0' || str[1] == '/')
	{
		result = join_strings(home, str + 1);
		return (result);
	}
	
	return (safe_strdup(str));
}

static int	match_pattern(char *str, char *pattern)
{
	if (*pattern == '\0')
		return (*str == '\0');
	
	if (*pattern == '*')
	{
		/* Try matching zero characters */
		if (match_pattern(str, pattern + 1))
			return (1);
		
		/* Try matching one or more characters */
		if (*str != '\0' && match_pattern(str + 1, pattern))
			return (1);
		
		return (0);
	}
	
	if (*str == '\0')
		return (0);
	
	if (*pattern == '?' || *pattern == *str)
		return (match_pattern(str + 1, pattern + 1));
	
	return (0);
}

char	**expand_wildcards(char *pattern)
{
	DIR				*dir;
	struct dirent	*entry;
	char			**matches;
	int				count;
	int				capacity;

	dir = opendir(".");
	if (!dir)
		return (NULL);
	
	capacity = 10;
	matches = safe_malloc(sizeof(char *) * capacity);
	count = 0;
	
	while ((entry = readdir(dir)) != NULL)
	{
		/* Skip hidden files unless pattern starts with . */
		if (entry->d_name[0] == '.' && pattern[0] != '.')
			continue;
		
		if (match_pattern(entry->d_name, pattern))
		{
			if (count >= capacity - 1)
			{
				capacity *= 2;
				matches = realloc(matches, sizeof(char *) * capacity);
				if (!matches)
					exit_error("realloc failed");
			}
			matches[count] = safe_strdup(entry->d_name);
			count++;
		}
	}
	
	closedir(dir);
	
	if (count == 0)
	{
		free(matches);
		return (NULL);
	}
	
	matches[count] = NULL;
	return (matches);
}
