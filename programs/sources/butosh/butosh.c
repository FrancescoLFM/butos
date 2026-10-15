/*
    butosh - Butos shell
    First process started by the kernel: runs builtins and launches the
    programs on the FAT32 partition through the exec syscall.
*/
#include <include/syscalls.h>
#include "ulib.h"

#define BUFFER_SIZE     128
#define PATH_SIZE       (BUFFER_SIZE + 16)
/* Same as the kernel PATH_MAX */
#define CWD_SIZE        256
#define MAX_ARGS        16
#define VAR_SIZE        50
#define NO_BUILTIN      -1

/* VGA colors */
#define BLACK           0b000
#define BLUE            0b001
#define GREEN           0b010
#define WHITE           0b111
#define STD_COLOR       0b00000111

struct command {
    char *name;
    char *args[MAX_ARGS];
    size_t argc;
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

    for (size_t i = 0; i < var_count; i++)
        printf_syscall("%s: %s\n", vars[i].name, vars[i].value);

    return EXIT_SUCCESS;
}

static int print_program(char **args, size_t argc)
{
    for (size_t i = 0; i < argc; i++) {
        puts_syscall(args[i]);
        putc_syscall(' ');
    }
    putc_syscall('\n');

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
    printf_syscall("%s\n", cwd);

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

/* Splits input in place on spaces, $name arguments are replaced by the variable value */
static int command_tokenize(char *input, struct command *cmd)
{
    char *token, *resolved_val;

    cmd->name = NULL;
    cmd->argc = 0;
    while (*input) {
        while (*input == ' ')
            *input++ = '\0';
        if (*input == '\0')
            break;
        token = input;
        while (*input && *input != ' ')
            input++;
        if (*input)
            *input++ = '\0';

        if (cmd->name == NULL) {
            cmd->name = token;
            continue;
        }
        if (cmd->argc == MAX_ARGS) {
            puts_syscall("Too many arguments\n");
            return EXIT_FAILURE;
        }
        if (token[0] == '$' && token[1] != '\0') {
            resolved_val = var_resolve(token + 1);
            if (resolved_val == NULL)
                printf_syscall("No variable named %s, fallbacking\n", token + 1);
            else
                token = resolved_val;
        }
        cmd->args[cmd->argc++] = token;
    }

    return EXIT_SUCCESS;
}

static int builtin_program_execute(struct command *cmd)
{
    for (size_t i = 0; i < ARR_SIZE(builtin_commands); i++)
        if (strcmp(cmd->name, builtin_commands[i].name) == 0)
            return builtin_commands[i].builtin_program(cmd->args, cmd->argc);

    return NO_BUILTIN;
}

/* Programs are searched in /usr/bin, /bin, / unless a path is given (e.g. /prog, ./prog, dir/prog),
   relative paths start from the working directory */
static int program_execute(struct command *cmd)
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
        status = exec_syscall(path, argv, &exit_code);
    } else {
        for (i = 0; search_dirs[i] != NULL && status == EXEC_NOT_FOUND; i++) {
            size_t prefix_len = strlen(search_dirs[i]);
            if (prefix_len + len + 1 > PATH_SIZE)
                continue;
            memcpy(path, search_dirs[i], prefix_len);
            memcpy(path + prefix_len, cmd->name, len + 1);
            status = exec_syscall(path, argv, &exit_code);
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
        printf_syscall("%s: arguments too long\n", cmd->name);
        break;
    default:
        printf_syscall("%s: failed to execute\n", cmd->name);
        break;
    }

    return EXIT_FAILURE;
}

static int butosh_interpret(char *input)
{
    struct command cmd;
    int ret;

    if (command_tokenize(input, &cmd) || cmd.name == NULL)
        return EXIT_FAILURE;
    if ((ret = builtin_program_execute(&cmd)) != NO_BUILTIN)
        return ret;

    return program_execute(&cmd);
}

static void shell_prompt()
{
    char *cwd = var_resolve("PWD");

    puts_color_syscall(shell_color, "butosh ");
    puts_color_syscall(shell_color, cwd != NULL ? cwd : "?");
    puts_color_syscall(shell_color, " $ ");
}

static void shell_scan(char *buffer)
{
    char c;
    size_t len = 0;

    for (;;) {
        getchar_syscall(&c);
        if (c == '\n')
            break;
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
        shell_scan(buffer);
        butosh_interpret(buffer);
    }

    free(buffer);
    for (size_t i = 0; i < var_count; i++) {
        free(vars[i].name);
        free(vars[i].value);
    }

    return shell_exit_code;
}
