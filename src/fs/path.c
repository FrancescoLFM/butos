#include <fs/path.h>
#include <libs/string.h>

/* Appends the components of path to out, which always holds an absolute path of len chars */
static int path_append(char *out, size_t *len, size_t size, const char *path)
{
    const char *comp;
    size_t comp_len;

    for (;;) {
        while (*path == '/')
            path++;
        if (*path == '\0')
            return EXIT_SUCCESS;
        comp = path;
        while (*path && *path != '/')
            path++;
        comp_len = (size_t) (path - comp);

        if (comp_len == 1 && comp[0] == '.')
            continue;
        if (comp_len == 2 && comp[0] == '.' && comp[1] == '.') {
            while (*len > 1 && out[*len - 1] != '/')
                (*len)--;
            if (*len > 1)
                (*len)--;
            continue;
        }

        /* Separator, component and terminator */
        if (*len + (*len > 1) + comp_len + 1 > size)
            return EXIT_FAILURE;
        if (*len > 1)
            out[(*len)++] = '/';
        memcpy(out + *len, comp, comp_len);
        *len += comp_len;
    }
}

int path_resolve(const char *cwd, const char *path, char *out, size_t size)
{
    size_t len = 1;

    if (cwd == NULL || path == NULL || *path == '\0' || out == NULL || size < 2)
        return EXIT_FAILURE;

    out[0] = '/';
    if (*path != '/' && path_append(out, &len, size, cwd))
        return EXIT_FAILURE;
    if (path_append(out, &len, size, path))
        return EXIT_FAILURE;
    out[len] = '\0';

    return EXIT_SUCCESS;
}
