#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE_BUFFER 1024
#define CLOSE(fd1, fd2)         \
    if (close (fd1) < 0) {      \
        perror ("Error close"); \
        return errno;           \
    }                           \
    if (close (fd2) < 0) {      \
        perror ("Error close"); \
        return errno;           \
    }

//=====================================================================================================

char fill_options (int argc, char *argv[]);

int copy (int fd1, int fd2);

char *create_file_name (const char *dir_name, const char *file_name);

int subject_type (const char *name);  // 0 	=> file exist
                                      // 1 	=> is directory
                                      // -1 	=> doesn't exist

//=====================================================================================================

int main (int argc, char *argv[])
{
    if (argc == 1) {
        printf ("my_cp: missing file operand\n");
        return 0;
    }

    char options = fill_options (argc, argv);  // last bit is flag for "-f" option
                                               // 2nd from end bit is flag for "-i" option
                                               // 3rd from end bit is flag for "-v" option
    int pos_files = 1;

    for (; argv[pos_files][0] == '-'; ++pos_files)
        ;

    if (argc - 1 - pos_files < 1) {
        printf ("my_cp: missing destination file operand after '%s'\n", argv[argc - 1]);
        return 0;
    }

    if (argc - 1 - pos_files > 1) {
        int dir_is = subject_type (argv[argc - 1]);
        if (dir_is != 1) {
            printf ("my_cp: target '%s' is not a directory\n", argv[argc - 1]);
            return 0;
        }

        for (int i = pos_files; i < argc - 1; ++i) {
            int fd1 = open (argv[i], O_RDWR);
            if (fd1 < 0) {
                perror ("Error open read");
                return errno;
            }

            char *full_name = create_file_name (argv[argc - 1], argv[i]);
            if ((options >> 1) & 1)
                if (!subject_type (full_name)) {  // means that file already exists
                    printf ("my_cp: overwrite \'%s\'? ", full_name);
                    if (getchar () != 'y') {
                        while (getchar () != '\n')
                            ;
                        continue;
                    }
                    while (getchar () != '\n')
                        ;
                }

            int fd2 = open (full_name, O_CREAT | O_WRONLY | O_TRUNC, 0777);
            if (fd2 < 0) {
                if (options & 1) {
                    if (unlink (full_name)) {
                        perror ("Error open read");
                        return errno;
                    }
                    fd2 = open (full_name, O_CREAT | O_WRONLY | O_TRUNC, 0777);
                }
                else {
                    perror ("Error open write");
                    continue;
                }
            }

            if ((options >> 2) & 1)
                printf ("\'%s\' -> \'%s\'\n", argv[i], full_name);

            free (full_name);

            if (copy (fd1, fd2) < 0) {
                perror ("Error copy");
                return errno;
            }
            CLOSE (fd1, fd2);
        }
        return 0;
    }
    else {
        int fd1 = open (argv[pos_files], O_RDONLY);
        if (fd1 < 0) {
            perror ("Error open read");
            return errno;
        }

        int dir_is = subject_type (argv[argc - 1]);
        char *full_name;
        if (dir_is == 1)
            full_name = create_file_name (argv[argc - 1], argv[pos_files]);
        else
            full_name = argv[argc - 1];

        if ((options >> 1) & 1)
            if (!subject_type (full_name)) {  // means that file already exists
                printf ("my_cp: overwrite \'%s\'? ", full_name);
                if (getchar () != 'y') {
                    while (getchar () != '\n')
                        ;
                    return 0;
                }
                while (getchar () != '\n')
                    ;
            }

        int fd2 = open (full_name, O_CREAT | O_WRONLY | O_TRUNC, 0777);
        if (fd2 < 0) {
            if (options & 1) {
                if (unlink (full_name)) {
                    perror ("Error open read");
                    return errno;
                }
                fd2 = open (full_name, O_CREAT | O_WRONLY | O_TRUNC, 0777);
            }
            else {
                perror ("Error open write");
                return errno;
            }
        }

        if ((options >> 2) & 1)
            printf ("\'%s\' -> \'%s\'\n", argv[pos_files], full_name);

        if (dir_is)
            free (full_name);

        if (copy (fd1, fd2) < 0) {
            perror ("Error copy");
            return errno;
        }
        CLOSE (fd1, fd2);
    }

    return 0;
}

//=====================================================================================================

char fill_options (int argc, char *argv[])
{
    struct option longopts[] = {{"force", 0, 0, 'f'}, {"interactive", 0, 0, 'i'}, {"verbose", 0, 0, 'v'}, {0, 0, 0, 0}};

    int c;
    char flags = 0;  // last bit is flag for "-f" option
                     // 2nd from end bit is flag for "-i" option
                     // 3rd from end bit is flag for "-v" option

    while ((c = getopt_long (argc, argv, "fiv", longopts, NULL)) != -1) {
        switch (c) {
            case 'f': flags |= 1; break;
            case 'i': flags |= 1 << 1; break;
            case 'v': flags |= 1 << 2; break;
            default: return 0;
        }
    }

    return flags;
}

//-----------------------------------------------------------------------------------------------------

int copy (int fd1, int fd2)
{
    char str[SIZE_BUFFER];

    while (1) {
        int end = read (fd1, str, SIZE_BUFFER);
        if (end < 0) {
            perror ("Error read");
            return errno;
        }
        if (!end)
            break;

        int position = 0;
        while (position < end) {
            int wr_count = write (fd2, str + position, end - position);
            if (wr_count < 0) {
                perror ("Error write");
                return errno;
            }
            position += wr_count;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------------------------------

char *create_file_name (const char *dir_name, const char *file_name)
{
    size_t name_dir_len = strlen (dir_name);
    int slash = 0;

    if (dir_name[name_dir_len - 1] != '/') {
        slash = 1;
    }
    int begin_name_file = strlen (file_name) - 1;

    for (; begin_name_file > 0 && file_name[begin_name_file] != '/'; --begin_name_file)
        ;

    size_t len = name_dir_len + strlen (file_name) - begin_name_file + 1 + slash;

    char *name = (char *)malloc (len * sizeof (char));

    strcat (name, dir_name);

    if (slash)
        strcat (name, "/");

    strcat (name, file_name + begin_name_file);

    return name;
}

//-----------------------------------------------------------------------------------------------------

int subject_type (const char *name)
{
    struct stat info;
    int stat_error = stat (name, &info);
    if (stat_error < 0) {
        return stat_error;
    }

    return S_ISDIR (info.st_mode);
}
