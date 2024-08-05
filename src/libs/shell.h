#define BUFFER_SIZE 50
#define VAR_SIZE    50

int shell_init();
#define GENERIC_ERROR       1
#define NO_BUILTIN_PROGRAM  -1

#define BUILTIN_COMMAND(NAME, FUN) ((struct builtin_command){.name = NAME, .builtin_program = FUN})

struct command {
    char *name;         //* Can't be NULL
    char **args;        //* Nullable
    size_t argc;
};

struct builtin_command {
    char *name;
    int (*builtin_program)(char **, size_t);
};

struct var {
    char *name;
    char *value;
    uint8_t is_number;
};

struct var_array {
    struct var **array;
    size_t size;
    size_t maxsize;
};

int print_program(char **arg, size_t argc); /* The equivalent of echo */
int set_var_str(char **args, size_t argc); /* Set a local string variable */
int set_var_int(char **args, size_t argc); /* Set a local integer variable */
int clear_command(char **arg, size_t argc); /* Clear that can change bg color */
int setcolor_command(char **args, size_t argc); /* Change shell prompt color */
int print_vars(char **args, size_t argc);
//...
static const struct builtin_command builtin_commands[] = {BUILTIN_COMMAND("print", print_program), 
                                                          BUILTIN_COMMAND("set", set_var_str), 
                                                          BUILTIN_COMMAND("let", set_var_int), 
                                                          BUILTIN_COMMAND("clear", clear_command), 
                                                          BUILTIN_COMMAND("setcolor", setcolor_command),
                                                          BUILTIN_COMMAND("printvars", print_vars)};

struct command* command_alloc(char *cmd_name_org, char **args_org, size_t argc);
void command_dealloc(struct command *cmd);
struct command* command_tokenize(char input[]);
int builtin_program_execute(struct command *cmd); //! Temporary
int command_execute(struct command *cmd);