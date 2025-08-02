#include "../include/minishell.h"

static t_env	*create_env_node(char *key, char *value)
{
	t_env	*node;

	node = safe_malloc(sizeof(t_env));
	node->key = safe_strdup(key);
	node->value = value ? safe_strdup(value) : safe_strdup("");
	node->next = NULL;
	return (node);
}

static void	add_env_node(t_env **env_list, t_env *new_node)
{
	t_env	*current;

	if (!*env_list)
	{
		*env_list = new_node;
		return ;
	}
	current = *env_list;
	while (current->next)
		current = current->next;
	current->next = new_node;
}

void	init_env(char **envp)
{
	char	*equals;
	char	*key;
	char	*value;
	int		i;

	g_shell.env_list = NULL;
	
	if (!envp)
		return ;
	
	i = 0;
	while (envp[i])
	{
		equals = strchr(envp[i], '=');
		if (equals)
		{
			*equals = '\0';
			key = envp[i];
			value = equals + 1;
			add_env_node(&g_shell.env_list, create_env_node(key, value));
			*equals = '=';  // Restore original string
		}
		i++;
	}
	
	update_env_array();
}

char	*get_env_value(char *key)
{
	t_env	*current;

	if (!key)
		return (NULL);
	
	current = g_shell.env_list;
	while (current)
	{
		if (strcmp(current->key, key) == 0)
			return (current->value);
		current = current->next;
	}
	
	return (NULL);
}

int	set_env_value(char *key, char *value)
{
	t_env	*current;

	if (!key)
		return (0);
	
	/* Search for existing variable */
	current = g_shell.env_list;
	while (current)
	{
		if (strcmp(current->key, key) == 0)
		{
			free(current->value);
			current->value = value ? safe_strdup(value) : safe_strdup("");
			update_env_array();
			return (1);
		}
		current = current->next;
	}
	
	/* Add new variable */
	add_env_node(&g_shell.env_list, create_env_node(key, value));
	update_env_array();
	return (1);
}

int	unset_env_value(char *key)
{
	t_env	*current;
	t_env	*prev;

	if (!key)
		return (0);
	
	current = g_shell.env_list;
	prev = NULL;
	
	while (current)
	{
		if (strcmp(current->key, key) == 0)
		{
			if (prev)
				prev->next = current->next;
			else
				g_shell.env_list = current->next;
			
			free(current->key);
			free(current->value);
			free(current);
			update_env_array();
			return (1);
		}
		prev = current;
		current = current->next;
	}
	
	return (0);
}

void	update_env_array(void)
{
	t_env	*current;
	int		count;
	int		i;
	char	*temp;

	/* Free old array */
	if (g_shell.env_array)
		free_string_array(g_shell.env_array);
	
	/* Count environment variables */
	count = 0;
	current = g_shell.env_list;
	while (current)
	{
		count++;
		current = current->next;
	}
	
	/* Allocate new array */
	g_shell.env_array = safe_malloc(sizeof(char *) * (count + 1));
	
	/* Fill array */
	i = 0;
	current = g_shell.env_list;
	while (current)
	{
		temp = join_strings(current->key, "=");
		g_shell.env_array[i] = join_strings(temp, current->value);
		free(temp);
		current = current->next;
		i++;
	}
	g_shell.env_array[i] = NULL;
}

void	free_env(void)
{
	t_env	*current;
	t_env	*next;

	current = g_shell.env_list;
	while (current)
	{
		next = current->next;
		free(current->key);
		free(current->value);
		free(current);
		current = next;
	}
	
	if (g_shell.env_array)
		free_string_array(g_shell.env_array);
}
