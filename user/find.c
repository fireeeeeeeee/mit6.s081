#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
char *fmtname(char *path)
{
    char *p;
    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    return p;
}

void find(char *path, char *tar)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:
        if (strcmp(fmtname(path), tar) == 0)
        {
            printf("%s\n", path);
        }
        break;

    case T_DIR:

        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0)
                continue;
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            {
                continue;
            }
            memmove(p, de.name, sizeof(de.name));
            p[sizeof(de.name)] = 0;
            if (stat(buf, &st) < 0)
            {
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            find(buf, tar);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
   
    if (argc < 2 || argc > 3)
    {
        printf("argument error\n");
        exit(0);
    }
    if (argc == 2)
    {
        find(".", argv[1]);
        exit(0);
    }

    if (argc == 3)
    {
        find(argv[1], argv[2]);
    }
    exit(0);
}

