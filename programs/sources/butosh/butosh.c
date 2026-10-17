/*
    butosh - Butos shell
    First process started by the kernel: runs builtins and launches the
    programs on the FAT32 partition through the exec syscall.
*/
#include <include/syscalls.h>
#include <libs/string.h>
#include <libs/atoi.h>
#include "ulib.h"

#define BUFFER_SIZE     128
#define PATH_SIZE       (BUFFER_SIZE + 16)
/* Same as the kernel PATH_MAX */
#define CWD_SIZE        256
#define MAX_ARGS        16
#define VAR_SIZE        50

/* VGA colors */
#define BLACK           0b000
#define BLUE            0b001
#define GREEN           0b010
#define WHITE           0b111
#define STD_COLOR       0b00000111

/* Ctrl+D, what getchar returns at the end of a redirected input */
#define EOF_CHAR        0x04

/* Every token takes at least one character of the line */
#define MAX_TOKENS      BUFFER_SIZE
#define MAX_PIPELINE    8

typedef enum {
    TOKEN_WORD,
    TOKEN_PIPE,             /* | */
    TOKEN_REDIRECT_IN,      /* < */
    TOKEN_REDIRECT_OUT,     /* > */
    TOKEN_REDIRECT_APPEND,  /* >> */
} token_type_t;

struct token {
    token_type_t type;
    /* TOKEN_WORD only: points into the input line */
    char *str;
};

struct command {
    char *name;
    char *args[MAX_ARGS];
    size_t argc;
    /* Redirections, NULL to use the streams of the pipeline */
    char *file_in;
    char *file_out;
    /* file_out is opened with >> */
    int append;
};

struct builtin_command {
    char *name;
    int (*builtin_program)(char **, size_t);
};

struct var {
    char *name;
    char *value;
    u8 is_number;
};

static struct var vars[VAR_SIZE];
static size_t var_count;
static u8 shell_color = GREEN;
/* Set by the exit builtin */
static int shell_running = 1;
static int shell_exit_code = EXIT_SUCCESS;
/* Where the running builtin writes its output: stdout, a file or a pipe */
static int builtin_out = STDOUT_FILENO;

static struct var *var_search(char *name)
{
    for (size_t i = 0; i < var_count; i++)
        if (strcmp(vars[i].name, name) == 0)
            return &vars[i];

    return NULL;
}

static char *var_resolve(char *name)
{
    struct var *v = var_search(name);

    return v != NULL ? v->value : NULL;
}

static int var_set(char *name, char *value, u8 is_number)
{
    struct var *var;
    char *value_copy;

    value_copy = strdup(value);
    if (value_copy == NULL)
        return EXIT_FAILURE;

    /* Sovrascrivi se esistente */
    if ((var = var_search(name)) != NULL) {
        free(var->value);
    } else {
        if (var_count == VAR_SIZE) {
            puts_syscall("Too many variables\n");
            free(value_copy);
            return EXIT_FAILURE;
        }
        var = &vars[var_count];
        var->name = strdup(name);
        if (var->name == NULL) {
            free(value_copy);
            return EXIT_FAILURE;
        }
        var_count++;
    }
    var->value = value_copy;
    var->is_number = is_number;

    return EXIT_SUCCESS;
}

static void builtin_puts(char *str)
{
    write_syscall(builtin_out, str, strlen(str));
}

/* Builtin commands section start */
static int set_var_str(char **args, size_t argc)
{
    if (argc < 2) {
        puts_syscall("set usage: set [var] [value]\n");
        return EXIT_FAILURE;
    }

    return var_set(args[0], args[1], 0);
}

static int set_var_int(char **args, size_t argc)
{
    if (argc < 2) {
        puts_syscall("let usage: let [var] [value]\n");
        return EXIT_FAILURE;
    }

    return var_set(args[0], args[1], 1);
}

static int print_vars(char **args, size_t argc)
{
    if (argc > 0) {
        printf_syscall("Unsupported argument: %s\n", args[0]);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < var_count; i++) {
        builtin_puts(vars[i].name);
        builtin_puts(": ");
        builtin_puts(vars[i].value);
        builtin_puts("\n");
    }

    return EXIT_SUCCESS;
}

static int print_program(char **args, size_t argc)
{
    for (size_t i = 0; i < argc; i++) {
        if (i > 0)
            builtin_puts(" ");
        builtin_puts(args[i]);
    }
    builtin_puts("\n");

    return EXIT_SUCCESS;
}

static int clear_command(char **args, size_t argc)
{
    u8 bg_color = BLACK;

    if (argc > 0) {
        if (strcmp(args[0], "macos") == 0)
            bg_color = STD_COLOR;
        else if (strcmp(args[0], "windows") == 0)
            bg_color = BLUE;
        else
            bg_color = (u8) atoi(args[0]) & WHITE;
    }
    clear_syscall(bg_color);

    return EXIT_SUCCESS;
}

static int setcolor_command(char **args, size_t argc)
{
    if (argc == 0) {
        puts_syscall("setcolor usage: setcolor [color]\n");
        return EXIT_FAILURE;
    }
    shell_color = (u8) atoi(args[0]) & WHITE;

    return EXIT_SUCCESS;
}

/* Keeps $PWD in sync with the kernel working directory */
static int pwd_update(void)
{
    char cwd[CWD_SIZE];

    if (getcwd_syscall(cwd, sizeof(cwd)) < 0)
        return EXIT_FAILURE;

    return var_set("PWD", cwd, 0);
}

static int cd_command(char **args, size_t argc)
{
    char *path = argc > 0 ? args[0] : "/";

    if (argc > 1) {
        puts_syscall("cd usage: cd [dir]\n");
        return EXIT_FAILURE;
    }
    switch (chdir_syscall(path)) {
    case FILE_OK:
        return pwd_update();
    case FILE_NOT_FOUND:
        printf_syscall("cd: %s: no such directory\n", path);
        break;
    default:
        printf_syscall("cd: %s: not a directory\n", path);
        break;
    }

    return EXIT_FAILURE;
}

static int pwd_command(char **args, size_t argc)
{
    char cwd[CWD_SIZE];

    (void) args;
    if (argc > 0) {
        puts_syscall("pwd usage: pwd\n");
        return EXIT_FAILURE;
    }
    if (getcwd_syscall(cwd, sizeof(cwd)) < 0) {
        puts_syscall("pwd: failed to get the working directory\n");
        return EXIT_FAILURE;
    }
    builtin_puts(cwd);
    builtin_puts("\n");

    return EXIT_SUCCESS;
}

static int exit_command(char **args, size_t argc)
{
    shell_exit_code = argc > 0 ? atoi(args[0]) : EXIT_SUCCESS;
    shell_running = 0;

    return EXIT_SUCCESS;
}
/* Builtin commands section end */

static const struct builtin_command builtin_commands[] = {
    {"print", print_program},       /* The equivalent of echo */
    {"set", set_var_str},           /* Set a local string variable */
    {"let", set_var_int},           /* Set a local integer variable */
    {"clear", clear_command},       /* Clear that can change bg color */
    {"setcolor", setcolor_command}, /* Change shell prompt color */
    {"printvars", print_vars},
    {"cd", cd_command},             /* Change the working directory */
    {"pwd", pwd_command},           /* Print the working directory */
    {"exit", exit_command},
};

static int is_blank(char c)
{
    return c == ' ' || c == '\t';
}

static int is_operator(char c)
{
    return c == '|' || c == '<' || c == '>';
}

/* Splits input in place: words are separated by blanks or by the operators | < > >>,
   which are tokens even without blanks around them. Returns the token count */
static int tokenize(char *input, struct token *tokens)
{
    struct token *token;
    int count = 0;

    while (*input) {
        if (is_blank(*input)) {
            *input++ = '\0';
            continue;
        }

        token = &tokens[count++];
        token->str = NULL;
        switch (*input) {
        case '|':
            token->type = TOKEN_PIPE;
            break;
        case '<':
            token->type = TOKEN_REDIRECT_IN;
            break;
        case '>':
            token->type = TOKEN_REDIRECT_OUT;
            if (input[1] == '>') {
                token->type = TOKEN_REDIRECT_APPEND;
                *input++ = '\0';
            }
            break;
        default:
            /* Terminated when the blank or operator after it is consumed */
            token->type = TOKEN_WORD;
            token->str = input;
            while (*input && !is_blank(*input) && !is_operator(*input))
                input++;
            continue;
        }
        *input++ = '\0';
    }

    return count;
}

static char *token_name(const struct token *token)
{
    switch (token->type) {
    case TOKEN_PIPE:
        return "|";
    case TOKEN_REDIRECT_IN:
        return "<";
    case TOKEN_REDIRECT_OUT:
        return ">";
    case TOKEN_REDIRECT_APPEND:
        return ">>";
    default:
        return token->str;
    }
}

/* $name words are replaced by the variable value */
static char *word_expand(char *word)
{
    char *value;

    if (word[0] != '$' || word[1] == '\0')
        return word;
    value = var_resolve(word + 1);
    if (value == NULL) {
        printf_syscall("No variable named %s, fallbacking\n", word + 1);
        return word;
    }

    return value;
}

/* Groups the tokens into the commands of a pipeline: returns how many (0 for an empty line)
   or -1 on a syntax error */
static int parse_tokens(const struct token *tokens, int count, struct command *cmds)
{
    struct command *cmd = cmds;
    /* File name of the redirection waiting for its word */
    char **redirect = NULL;
    int cmd_count = 1;

    if (count == 0)
        return 0;

    memset(cmd, 0, sizeof(*cmd));
    for (int i = 0; i < count; i++) {
        const struct token *token = &tokens[i];

        if (redirect != NULL && token->type != TOKEN_WORD) {
            printf_syscall("Syntax error: file name expected before %s\n", token_name(token));
            return -1;
        }

        switch (token->type) {
        case TOKEN_WORD:
            if (redirect != NULL) {
                *redirect = word_expand(token->str);
                redirect = NULL;
            } else if (cmd->name == NULL) {
                cmd->name = word_expand(token->str);
            } else if (cmd->argc < MAX_ARGS) {
                cmd->args[cmd->argc++] = word_expand(token->str);
            } else {
                puts_syscall("Too many arguments\n");
                return -1;
            }
            break;
        case TOKEN_REDIRECT_IN:
            redirect = &cmd->file_in;
            break;
        case TOKEN_REDIRECT_OUT:
        case TOKEN_REDIRECT_APPEND:
            cmd->append = token->type == TOKEN_REDIRECT_APPEND;
            redirect = &cmd->file_out;
            break;
        case TOKEN_PIPE:
            if (cmd->name == NULL) {
                puts_syscall("Syntax error: command expected before |\n");
                return -1;
            }
            if (cmd_count == MAX_PIPELINE) {
                puts_syscall("Too many commands in the pipeline\n");
                return -1;
            }
            cmd = &cmds[cmd_count++];
            memset(cmd, 0, sizeof(*cmd));
            break;
        }
    }

    if (redirect != NULL) {
        puts_syscall("Syntax error: file name expected at the end of the line\n");
        return -1;
    }
    if (cmd->name == NULL) {
        puts_syscall("Syntax error: command expected\n");
        return -1;
    }

    return cmd_count;
}

static const struct builtin_command *builtin_search(char *name)
{
    for (size_t i = 0; i < ARR_SIZE(builtin_commands); i++)
        if (strcmp(name, builtin_commands[i].name) == 0)
            return &builtin_commands[i];

    return NULL;
}

/* Programs are searched in /usr/bin, /bin, / unless a path is given (e.g. /prog, ./prog, dir/prog),
   relative paths start from the working directory. stdio are its stdin, stdout, stderr */
static int program_execute(struct command *cmd, int *stdio)
{
    static const char * const search_dirs[] = {
        "/usr/bin/", "/bin/", "/", NULL
    };
    char path[PATH_SIZE];
    char *argv[MAX_ARGS + 2];
    int exit_code = 0;
    int status = EXEC_NOT_FOUND;
    size_t len = strlen(cmd->name);
    int i;

    argv[0] = cmd->name;
    memcpy(&argv[1], cmd->args, cmd->argc * sizeof(char *));
    argv[cmd->argc + 1] = NULL;

    if (strchr(cmd->name, '/') != NULL) {
        if (len + 1 > PATH_SIZE) {
            puts_syscall("Path too long\n");
            return EXIT_FAILURE;
        }
        memcpy(path, cmd->name, len + 1);
        status = exec_syscall(path, argv, stdio, &exit_code);
    } else {
        for (i = 0; search_dirs[i] != NULL && status == EXEC_NOT_FOUND; i++) {
            size_t prefix_len = strlen(search_dirs[i]);
            if (prefix_len + len + 1 > PATH_SIZE)
                continue;
            memcpy(path, search_dirs[i], prefix_len);
            memcpy(path + prefix_len, cmd->name, len + 1);
            status = exec_syscall(path, argv, stdio, &exit_code);
        }
    }

    switch (status) {
    case EXEC_OK:
        if (exit_code != EXIT_SUCCESS)
            printf_syscall("\n%s exited with code %d\n", cmd->name, exit_code);
        return exit_code;
    case EXEC_NOT_FOUND:
        printf_syscall("Unrecognized command: %s\n", cmd->name);
        break;
    case EXEC_INVALID:
        printf_syscall("%s: invalid executable\n", cmd->name);
        break;
    case EXEC_INVALID_ARGS:
        printf_syscall("%s: invalid arguments or redirections\n", cmd->name);
        break;
    default:
        printf_syscall("%s: failed to execute\n", cmd->name);
        break;
    }

    return EXIT_FAILURE;
}

static void redirect_error(char *path, int error)
{
    switch (error) {
    case FILE_NOT_FOUND:
        printf_syscall("%s: no such file\n", path);
        break;
    case FILE_INVALID:
        printf_syscall("%s: invalid file (names are 8.3)\n", path);
        break;
    case FILE_BUSY:
        printf_syscall("%s: file in use\n", path);
        break;
    default:
        printf_syscall("%s: can't open the file\n", path);
        break;
    }
}

/* Opens the file of an output redirection, creating it: > empties it, >> appends to it.
   Returns the descriptor or a FILE_* error */
static int redirect_open_out(char *path, int append)
{
    int fd, ret;

    /* There is no truncate: an existing file is replaced by an empty one */
    if (!append) {
        ret = remove_syscall(path);
        if (ret != FILE_OK && ret != FILE_NOT_FOUND)
            return ret;
    }
    ret = create_syscall(path);
    if (ret != FILE_OK && ret != FILE_EXISTS)
        return ret;

    fd = open_syscall(path);
    if (fd >= 0 && append && seek_syscall(fd, 0, SEEK_END) < 0) {
        close_syscall(fd);
        return FILE_IO_ERROR;
    }

    return fd;
}

/* Runs cmd reading from in and writing to out (the pipeline streams), unless it redirects them */
static int command_execute(struct command *cmd, int in, int out)
{
    const struct builtin_command *builtin;
    int stdio[] = {in, out, STDERR_FILENO};
    int ret = EXIT_FAILURE;

    if (cmd->file_in != NULL) {
        stdio[STDIN_FILENO] = open_syscall(cmd->file_in);
        if (stdio[STDIN_FILENO] < 0) {
            redirect_error(cmd->file_in, stdio[STDIN_FILENO]);
            return EXIT_FAILURE;
        }
    }
    if (cmd->file_out != NULL) {
        stdio[STDOUT_FILENO] = redirect_open_out(cmd->file_out, cmd->append);
        if (stdio[STDOUT_FILENO] < 0) {
            redirect_error(cmd->file_out, stdio[STDOUT_FILENO]);
            goto close_in;
        }
    }

    /* Builtins don't read their input */
    builtin = builtin_search(cmd->name);
    if (builtin != NULL) {
        builtin_out = stdio[STDOUT_FILENO];
        ret = builtin->builtin_program(cmd->args, cmd->argc);
        builtin_out = STDOUT_FILENO;
    } else {
        ret = program_execute(cmd, stdio);
    }

    /* Writes reach the disk on close */
    if (cmd->file_out != NULL && close_syscall(stdio[STDOUT_FILENO]) != FILE_OK) {
        printf_syscall("%s: failed to write the file\n", cmd->file_out);
        ret = EXIT_FAILURE;
    }
close_in:
    if (cmd->file_in != NULL)
        close_syscall(stdio[STDIN_FILENO]);

    return ret;
}

/* Programs run one at a time: each command writes its whole output to a pipe,
   then the next one reads it. Returns the exit code of the last command */
static int pipeline_execute(struct command *cmds, int count)
{
    /* Written by the previous command, -1 for the first one */
    int pipe_in = -1;
    int pipe_out;
    int ret = EXIT_FAILURE;

    for (int i = 0; i < count && shell_running; i++) {
        pipe_out = -1;
        if (i + 1 < count && (pipe_out = pipe_syscall()) < 0) {
            puts_syscall("Failed to open a pipe\n");
            ret = EXIT_FAILURE;
            break;
        }
        ret = command_execute(&cmds[i],
                              pipe_in >= 0 ? pipe_in : STDIN_FILENO,
                              pipe_out >= 0 ? pipe_out : STDOUT_FILENO);
        if (pipe_in >= 0)
            close_syscall(pipe_in);
        pipe_in = pipe_out;
    }
    if (pipe_in >= 0)
        close_syscall(pipe_in);

    return ret;
}

static int butosh_interpret(char *input)
{
    /* Static: processes run on the kernel stack */
    static struct token tokens[MAX_TOKENS];
    static struct command cmds[MAX_PIPELINE];
    int count;

    count = tokenize(input, tokens);
    count = parse_tokens(tokens, count, cmds);
    if (count <= 0)
        return count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;

    return pipeline_execute(cmds, count);
}

static void shell_prompt()
{
    char *cwd = var_resolve("PWD");

    puts_color_syscall(shell_color, "butosh ");
    puts_color_syscall(shell_color, cwd != NULL ? cwd : "?");
    puts_color_syscall(shell_color, " $ ");
}

/* Reads a line, returns 0 at the end of the input (Ctrl+D on an empty line) */
static int shell_scan(char *buffer)
{
    char c;
    size_t len = 0;

    for (;;) {
        getchar_syscall(&c);
        if (c == '\n')
            break;
        if (c == EOF_CHAR) {
            if (len == 0) {
                putc_syscall('\n');
                return 0;
            }
            break;
        }
        switch (c) {
        case '\b':
            if (len == 0)
                break;
            len--;
            putc_syscall(c);
            break;
        default:
            if (len >= BUFFER_SIZE - 1)
                break;
            buffer[len++] = c;
            putc_syscall(c);
            break;
        }
    }
    buffer[len] = '\0';
    putc_syscall('\n');

    return 1;
}

static void load_default_var()
{
    var_set("black", "0", 1);
    var_set("blue", "1", 1);
    var_set("green", "2", 1);
    var_set("red", "4", 1);
    var_set("white", "7", 1);
    if (pwd_update())
        var_set("PWD", "/", 0);
}

int main()
{
    /* Kernel heap memory: still valid while the programs it launches run */
    char *buffer = malloc(BUFFER_SIZE);

    if (buffer == NULL) {
        puts_syscall("Failed to allocate buffer\n");
        return EXIT_FAILURE;
    }
    load_default_var();
    while (shell_running) {
        shell_prompt();
        if (!shell_scan(buffer))
            break;
        butosh_interpret(buffer);
    }

    free(buffer);
    for (size_t i = 0; i < var_count; i++) {
        free(vars[i].name);
        free(vars[i].value);
    }

    return shell_exit_code;
}
