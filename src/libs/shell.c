#include <include/def.h>
#include <libs/print.h>
#include <libs/alloc.h>
#include <libs/scan.h>
#include <libs/shell.h>
#include <libs/string.h>

static struct var_array *var_array_container;

void var_array_alloc() {
    var_array_container = kalloc(sizeof(*var_array_container));
    if (var_array_container == NULL)
        return;
    var_array_container->array = kcalloc(VAR_SIZE, sizeof(*var_array_container->array));
    if (var_array_container->array == NULL) {
        kfree(var_array_container);
        var_array_container = NULL;
        return;
    }

    var_array_container->maxsize = VAR_SIZE;
}

void var_array_dealloc() {
    for (size_t i=0; i<var_array_container->size; i++) 
        if (var_array_container->array[i] != NULL) {
            kfree(var_array_container->array[i]->name);
            kfree(var_array_container->array[i]->value);
            kfree(var_array_container->array[i]);
        }
    kfree(var_array_container->array);
    kfree(var_array_container);
}

struct var *var_alloc() {
    if (var_array_container->size == var_array_container->maxsize) {
        var_array_container->maxsize *= 2;
        var_array_container->array = krealloc(var_array_container->array, var_array_container->maxsize);
        if (var_array_container->array == NULL)
            return NULL;
    }

    var_array_container->array[var_array_container->size] = kalloc(sizeof(**var_array_container->array));
    if (var_array_container->array[var_array_container->size] == NULL)
        return NULL;
 
    return var_array_container->array[var_array_container->size++];
}

/* Builtin commands section start */
int set_var_str(char **args, size_t argc) {
    // TODO Controlla se la variabile esiste già (sovrascrivila)
    struct var *var;

    if (argc < 2) {
        puts("set usage: set [var] [value]\n");
        return EXIT_FAILURE;
    }

    var = var_alloc();
    var->name = strdup(args[0]);
    var->value = strdup(args[1]);
    var->is_number = 0;

    return EXIT_SUCCESS;
}

/* Builtin commands section start */
int set_var_int(char **args, size_t argc) {
    // TODO Controlla se la variabile esiste già (sovrascrivila)
    struct var *var;

    if (argc < 2) {
        puts("set usage: set [var] [value]\n");
        return EXIT_FAILURE;
    }

    var = var_alloc();
    var->name = strdup(args[0]);
    var->value = strdup(args[1]);
    var->is_number = 1;

    return EXIT_SUCCESS;
}

int print_program(char **args, size_t argc) {
    for (size_t i=0; i < argc; i++) {
        puts(args[i]);
        putc(' ');
    }
    putc('\n');
    return 0;
}
/* Builtin commands section end */

//* Args can't be null
size_t args_get_count(char *args) {
    int count = 1;
    const char *temp = args;

    while ((temp = strchr(temp, ' ')) != NULL) {
        count++;
        temp++; 
    }

    return count;
}

char *var_resolve(char *name) {

    for (size_t i=0; i<var_array_container->size; i++)
        if(strcmp(var_array_container->array[i]->name, name) == 0) 
            return var_array_container->array[i]->value;

    return NULL;
}

//* Return value has to be kfreed
char **args_array_create(char args[], size_t argc) {
    char **args_array;
    char *resolved_val;

    if (args == NULL)
        return NULL;
    args_array = (char **) (kalloc(argc * sizeof(char *)));
    if (args_array == NULL)
        return NULL;

    for (size_t i=0; i <= argc; i++) {
        args_array[i] = strsep(&args, " ");
        if (*(args_array[i]) == '$') {
            if (*(args_array[i]+1) == '\0')
                continue;
    
            resolved_val = var_resolve(args_array[i]+1);
            if (resolved_val == NULL) {
                printk("No variable named %s, fallbacking\n", args_array[i]+1);
                continue;
            }
            args_array[i] = resolved_val;
        }
    }

    return args_array;
}

void args_array_destroy(char** args_array) {
    kfree(args_array);
}

//* Return value has to be kfreed with command_dealloc
struct command *command_alloc(char *cmd_name_org, char **args_org, size_t argc) {
    struct command *cmd;

    if (cmd_name_org == NULL)
        return NULL;
    cmd = (struct command *) (kalloc(sizeof(*cmd)));
    cmd->name = cmd_name_org;
    cmd->args = args_org;
    cmd->argc = argc;

    return cmd;
}

void command_dealloc(struct command *cmd) {
    if (cmd->args != NULL)
        args_array_destroy(cmd->args);
    kfree(cmd);
}

struct command* command_tokenize(char input[]) {
    struct command *cmd;
    size_t argc;

    if (input == NULL || input[0] == '\0')
        return NULL;
    
    char *cmd_name = strsep(&input, " ");
    argc = args_get_count(input);
    char **args = args_array_create(input, argc);
    cmd = command_alloc(cmd_name, args, argc);

    return cmd;
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
            return builtin_commands[i].builtin_program(cmd->args, cmd->argc);
    
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

    cmd = command_tokenize(input_raw);
    if (cmd == NULL)
        return EXIT_SUCCESS;
    errno = command_execute(cmd);

    //? OWNERSHIP PROBLEM?
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
    var_array_alloc();
    if (var_array_container == NULL) {
        shell_dealloc_buffer(buff);
        return EXIT_FAILURE;
    }
    for(;;) {
        shell_prompt();
        shell_scan(buff, rel_pos);
        butosh_interpret(buff);
    }
    /* Never reached */
    shell_dealloc_buffer(buff);
    return EXIT_SUCCESS;
}