# Program name
NAME = minishell

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj

# Source files
SOURCES = minishell.c \
          lexer.c \
          parser.c \
          executor.c \
          builtins.c \
          environment.c \
          utils.c \
          signals.c \
          heredoc.c \
          expansion.c

# Object files
SRCS = $(addprefix $(SRCDIR)/, $(SOURCES))
OBJS = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))

# Compiler and flags
CC = cc
CFLAGS = -Wall -Wextra -Werror -g
INCLUDES = -I$(INCDIR) -I/opt/homebrew/opt/readline/include
LIBS = -lreadline -L/opt/homebrew/opt/readline/lib

# Colors for pretty output
RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
BLUE = \033[0;34m
PURPLE = \033[0;35m
CYAN = \033[0;36m
WHITE = \033[0;37m
RESET = \033[0m

# Main target
all: $(NAME)

# Create object directory
$(OBJDIR):
	@mkdir -p $(OBJDIR)
	@echo "$(CYAN)Created object directory$(RESET)"

# Compile object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	@echo "$(YELLOW)Compiling $<$(RESET)"
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Link the program
$(NAME): $(OBJS)
	@echo "$(GREEN)Linking $(NAME)$(RESET)"
	@$(CC) $(OBJS) $(LIBS) -o $(NAME)
	@echo "$(GREEN)✓ $(NAME) created successfully!$(RESET)"

# Clean object files
clean:
	@if [ -d $(OBJDIR) ]; then \
		rm -rf $(OBJDIR); \
		echo "$(RED)Removed object files$(RESET)"; \
	fi

# Clean everything
fclean: clean
	@if [ -f $(NAME) ]; then \
		rm -f $(NAME); \
		echo "$(RED)Removed $(NAME)$(RESET)"; \
	fi

# Rebuild everything
re: fclean all

# Install readline (for macOS with Homebrew)
install-readline:
	@echo "$(CYAN)Installing readline...$(RESET)"
	@if command -v brew >/dev/null 2>&1; then \
		brew install readline; \
		echo "$(GREEN)Readline installed via Homebrew$(RESET)"; \
	else \
		echo "$(RED)Please install readline manually$(RESET)"; \
	fi

# Run the program
run: $(NAME)
	@echo "$(BLUE)Running $(NAME)$(RESET)"
	@./$(NAME)

# Debug with gdb
debug: $(NAME)
	@echo "$(PURPLE)Starting debugger$(RESET)"
	@gdb ./$(NAME)

# Memory check with valgrind
valgrind: $(NAME)
	@echo "$(PURPLE)Running valgrind$(RESET)"
	@valgrind --leak-check=full --show-leak-kinds=all ./$(NAME)

# Test basic functionality
test: $(NAME)
	@echo "$(CYAN)Running basic tests$(RESET)"
	@echo "echo 'Hello World'" | ./$(NAME)
	@echo "pwd" | ./$(NAME)
	@echo "env | head -5" | ./$(NAME)

# Show help
help:
	@echo "$(GREEN)Available targets:$(RESET)"
	@echo "  $(YELLOW)all$(RESET)              - Build the program"
	@echo "  $(YELLOW)clean$(RESET)            - Remove object files"
	@echo "  $(YELLOW)fclean$(RESET)           - Remove object files and executable"
	@echo "  $(YELLOW)re$(RESET)               - Rebuild everything"
	@echo "  $(YELLOW)run$(RESET)              - Build and run the program"
	@echo "  $(YELLOW)debug$(RESET)            - Build and run with gdb"
	@echo "  $(YELLOW)valgrind$(RESET)         - Build and run with valgrind"
	@echo "  $(YELLOW)test$(RESET)             - Run basic functionality tests"
	@echo "  $(YELLOW)install-readline$(RESET) - Install readline library"
	@echo "  $(YELLOW)help$(RESET)             - Show this help message"

# Declare phony targets
.PHONY: all clean fclean re install-readline run debug valgrind test help