#include <include/def.h>
#include <libs/print.h>
#include <libs/alloc.h>
#include <libs/scan.h>
#include <libs/shell.h>
#include <libs/string.h>

size_t args_get_count(char *args);

int print_program(char *args) {
    puts(args);
    putc('\n');
    return 0;
}

//* Return value has to be kfreed with command_dealloc
struct command* command_alloc(char *cmd_name_org, char *args_org) {
    struct command *cmd;

    if (cmd_name_org == NULL)
        return NULL;
    cmd = (struct command *) (kalloc(sizeof(*cmd)));
    cmd->name = strdup(cmd_name_org);
    if (args_org != NULL)
        cmd->args = strdup(args_org);

    return cmd;
}

void command_dealloc(struct command *cmd) {
    // if (cmd->args != NULL)
       //  kfree(cmd->args);
       //! OWNERSHIP PROBLEM
    kfree(cmd->name);
    kfree(cmd);
}

struct command* command_tokenize(char input[]) {
    struct command *cmd;

    if (input == NULL || input[0] == '\0')
        return NULL;
    
    char *cmd_name = strsep(&input, " ");
    char *args = input;

    cmd = command_alloc(cmd_name, args);

    return cmd;
}

//* Args can't be null
size_t args_get_count(char *args) {
    size_t count = 0;

    do {
        count++;
        args = strrchr(args, ' ')+1;
    } while (args-1 != NULL);
    
    return count;
}

//* Return value has to be kfreed
char **args_array_create(char args[]) {
    char **args_array;
    size_t array_size;

    if (args == NULL)
        return NULL;
    array_size = args_get_count(args);
    args_array = (char **) (kalloc(array_size * sizeof(char *)));

    for (size_t i=0; i <= array_size; i++)
        args_array[i] = strsep(&args, " ");

    return args_array;
}

void args_array_destroy(char** args_array) {
    kfree(args_array);
}

int parent_process_exec(char *path, char **argv) {
    //! DA IMPLEMENTARE!!!
    puts(path);
    puts(*argv);
    return 0;
}

int command_execute(struct command *cmd) {

    if (builtin_program_execute(cmd) != NO_BUILTIN_PROGRAM)
        return EXIT_SUCCESS;

    /* Implement program from path */

    printk("Unrecognized command: %s\n", cmd->name);
    return EXIT_FAILURE;
}

int builtin_program_execute(struct command *cmd) {
    size_t size = sizeof(builtin_commands) / sizeof(*builtin_commands);

    for (size_t i=0; i < size; i++)
        if (strcmp(cmd->name, builtin_commands[i].name) == 0)
            return builtin_commands[i].builtin_program(cmd->args);
    
    return NO_BUILTIN_PROGRAM;
}

void shell_prompt() {
    puts_c(GREEN, "$ ");
}

char *shell_alloc_buffer() {
    char *buffer;

    buffer = kalloc(BUFFER_SIZE * sizeof(*buffer));
    if (!buffer) {
        puts("Failed to allocate buffer\n");
        return NULL;
    }

    return buffer;
}

void shell_dealloc_buffer(char *buff) {
    kfree(buff);
}

void shell_scan(char *buffer, uint32_t rel_pos) {
    char c;

    while ((c = getchar()) != '\n') {
        switch (c) {
        case '\b':
            if (rel_pos == 0)
                break;
            rel_pos--, buffer--;
            putc(c);
            break;
        default:
            if (rel_pos >= BUFFER_SIZE)
                break;
            *buffer = c;
            buffer++, rel_pos++;
            putc(c);
            break;
        }
    }
    *buffer = '\0';
    putc('\n');
}

int butosh_interpret(char *input_raw) {
    struct command *cmd;
    int errno;
    char *input, *input_copy;

    input = strdup(input_raw);
    input_copy = input;
    cmd = command_tokenize(input);
    if (cmd == NULL)
        return EXIT_SUCCESS;
    errno = command_execute(cmd);
    //! OWNERSHIP PROBLEM 
    kfree(input_copy);
    command_dealloc(cmd);
    return errno;
}

int shell_init() {
    uint32_t rel_pos;
    char *buff;

    rel_pos = 0;
    buff = shell_alloc_buffer();
    if (buff == NULL)
        return EXIT_FAILURE;
    for(;;) {
        shell_prompt();
        shell_scan(buff, rel_pos);
        butosh_interpret(buff);
    }
    /* Never reached */
    shell_dealloc_buffer(buff);
    return EXIT_SUCCESS;
}