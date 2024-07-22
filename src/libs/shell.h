#define BUFFER_SIZE 50

int shell_init();
#define GENERIC_ERROR       1
#define NO_BUILTIN_PROGRAM  -1

#define BUILTIN_COMMAND(NAME, FUN) ((struct builtin_command){.name = NAME, .builtin_program = FUN})

struct command
{
    char *name;         //* Can't be NULL
    char *args;         //* Can be NULL
};

struct builtin_command
{
    char *name;
    int (*builtin_program)(char *);
};

int print_program(char *arg); /* The equivalent of echo */
//...
static const struct builtin_command builtin_commands[] = {BUILTIN_COMMAND("print", print_program)};

struct command* command_alloc(char *cmd_name_org, char *args_org);
void command_dealloc(struct command *cmd);
struct command* command_tokenize(char input[]);
int builtin_program_execute(struct command *cmd); //! Temporary
int command_execute(struct command *cmd);