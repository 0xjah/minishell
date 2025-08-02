#ifndef MINISHELL_H
# define MINISHELL_H

# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <signal.h>
# include <dirent.h>
# include <errno.h>
# include <fcntl.h>
# include <ctype.h>
# include <readline/readline.h>
# include <readline/history.h>

# define MAX_PATH 1024
# define MAX_ARGS 1024
# define MAX_ENV 1024

/* Token types */
typedef enum e_token_type
{
	TOKEN_WORD,
	TOKEN_PIPE,
	TOKEN_REDIRECT_IN,
	TOKEN_REDIRECT_OUT,
	TOKEN_REDIRECT_APPEND,
	TOKEN_REDIRECT_HEREDOC,
	TOKEN_AND,
	TOKEN_OR,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_EOF,
	TOKEN_ERROR
}	t_token_type;

/* Token structure */
typedef struct s_token
{
	t_token_type		type;
	char				*value;
	struct s_token		*next;
}	t_token;

/* Command structure */
typedef struct s_cmd
{
	char				**args;
	char				*input_file;
	char				*output_file;
	int					append_output;
	int					heredoc;
	char				*heredoc_delimiter;
	struct s_cmd		*next;
}	t_cmd;

/* Environment variable structure */
typedef struct s_env
{
	char				*key;
	char				*value;
	struct s_env		*next;
}	t_env;

/* Main shell structure */
typedef struct s_shell
{
	t_env				*env_list;
	char				**env_array;
	int					exit_status;
	int					stdin_backup;
	int					stdout_backup;
	pid_t				*pids;
	int					num_processes;
}	t_shell;

/* Global shell variable */
extern t_shell			g_shell;

/* Lexer functions */
t_token		*lexer(char *input);
t_token		*create_token(t_token_type type, char *value);
void		add_token(t_token **tokens, t_token *new_token);
void		free_tokens(t_token *tokens);
char		*extract_word(char *input, int *i);
char		*extract_quoted_string(char *input, int *i, char quote);

/* Parser functions */
t_cmd		*parser(t_token *tokens);
t_cmd		*create_cmd(void);
void		add_cmd(t_cmd **cmds, t_cmd *new_cmd);
void		free_cmds(t_cmd *cmds);
int			parse_command(t_token **tokens, t_cmd *cmd);
int			parse_redirections(t_token **tokens, t_cmd *cmd);

/* Executor functions */
int			executor(t_cmd *cmds);
int			execute_single_cmd(t_cmd *cmd);
int			execute_pipeline(t_cmd *cmds);
int			execute_builtin(t_cmd *cmd);
char		*find_command_path(char *cmd);

/* Built-in commands */
int			builtin_echo(char **args);
int			builtin_cd(char **args);
int			builtin_pwd(char **args);
int			builtin_export(char **args);
int			builtin_unset(char **args);
int			builtin_env(char **args);
int			builtin_exit(char **args);
int			is_builtin(char *cmd);

/* Environment functions */
void		init_env(char **envp);
char		*get_env_value(char *key);
int			set_env_value(char *key, char *value);
int			unset_env_value(char *key);
void		update_env_array(void);
void		free_env(void);

/* Signal handling */
void		setup_signals(void);
void		handle_sigint(int sig);
void		handle_sigquit(int sig);

/* Expansion functions */
char		*expand_variables(char *str);
char		*expand_tilde(char *str);
char		**expand_wildcards(char *pattern);

/* Utility functions */
char		**split_string(char *str, char delimiter);
char		*trim_whitespace(char *str);
char		*join_strings(char *s1, char *s2);
int			count_words(char *str, char delimiter);
void		free_string_array(char **array);
int			array_length(char **array);

/* Error handling */
void		print_error(char *cmd, char *msg);
void		exit_error(char *msg);
int			syntax_error(char *token);

/* File operations */
int			check_file_access(char *filename, int mode);
int			create_heredoc(char *delimiter);

/* Memory management */
void		*safe_malloc(size_t size);
char		*safe_strdup(char *str);
void		cleanup_shell(void);

#endif
