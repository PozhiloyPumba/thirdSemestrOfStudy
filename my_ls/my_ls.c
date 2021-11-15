#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

//=====================================================================================================

extern int optind;

//=====================================================================================================

int catch_bit (char opt, int shift);

char fill_options (int argc, char *argv[]);

int print_file(char *dir_name, char* name, char opts);

int ls_func (char *dir_name, char options);

int read_dir(DIR* dir, struct dirent** file);

//=====================================================================================================

int main(int argc, char *argv[]) {

    char options = fill_options(argc, argv);    //last bit is flag for "-a" option 
                                                //2nd from end bit is flag for "-l" option
                                                //3rd from end bit is flag for "-i" option
                                                //4rd from end bit is flag for "-n" option
                                                //5rd from end bit is flag for "-R" option

    if (optind == argc)
        ls_func("./", options);
    else
        ls_func(argv[optind], options);
    
    return 0;
}

//=====================================================================================================

int catch_bit (char opt, int shift) {

    return (opt >> shift) & 1;
}

//-----------------------------------------------------------------------------------------------------

char fill_options(int argc, char *argv[]) {

    struct option longopts[] = {
        {"all", 0, 0, 'a'},
        {"long", 0, 0, 'l'},
        {"inode", 0, 0, 'i'},
        {"numeric-uid-gid", 0, 0, 'n'},
        {"recursive", 0, 0, 'R'},
        {0, 0, 0, 0}
    };

    int c;
    char flags = 0;     //last bit is flag for "-a" option 
                        //2nd from end bit is flag for "-l" option
                        //3rd from end bit is flag for "-i" option
                        //4rd from end bit is flag for "-n" option
                        //5rd from end bit is flag for "-R" option


    while((c = getopt_long(argc, argv, "alinR", longopts, NULL)) != -1) {

        switch(c) {

            case 'a': flags |= 1;       break;
            case 'l': flags |= 1 << 1;  break;
            case 'i': flags |= 1 << 2;  break;
            case 'n': flags |= 5 << 1;  break;
            case 'R': flags |= 1 << 4;  break;
            default:
                return 0;
        }
    }

    return flags;
}

//-----------------------------------------------------------------------------------------------------

int print_file(char *dir_name, char* name, char opts){
    
    struct stat statbuf;

    size_t name_len = strlen(dir_name) + strlen(name) + 2;
    char *full_dir = (char *) malloc (sizeof (char) * name_len);
    full_dir[0] = 0;

    strcat(full_dir, dir_name);
    
    if (dir_name[strlen(dir_name) - 1] != '/')
        strcat(full_dir, "/");
    else --name_len;

    strcat(full_dir, name);

    full_dir[name_len - 1] = 0;
    
    if (catch_bit (opts, 2)) {

        lstat(full_dir, &statbuf);

        if (errno) {

            perror("lstat error");
            return -1;
        }
        printf("%lu ", statbuf.st_ino);
    }
    if (catch_bit (opts, 1)) {

        lstat(full_dir, &statbuf);

        if (errno) {

            perror("lstat error");
            return -1;
        }
        switch (statbuf.st_mode & S_IFMT) {

            case S_IFIFO:   printf("p"); break;
            case S_IFCHR:   printf("c"); break;
            case S_IFDIR:   printf("d"); break;
            case S_IFBLK:   printf("b"); break;
            case S_IFREG:   printf("-"); break;
            case S_IFLNK:   printf("l"); break;
            case S_IFSOCK:  printf("s"); break;
            default: printf("unknown?");
        }

        for(int i = 8; i >= 0; i--) {

            if (statbuf.st_mode & (1 << i)) {

                switch ((i + 1) % 3) {

                    case 0: printf("r"); break;
                    case 1: printf("x"); break;
                    case 2: printf("w"); break;
                    default: printf("WHAAAAAT????\n"); return -1;
                }
            } 
            else printf("-");
        }

        printf(" %lu ", statbuf.st_nlink);
        
        if (!catch_bit (opts, 3)) {
            
            errno = 0;
            struct passwd* pass = getpwuid(statbuf.st_uid);
            if (!errno)
                printf("%s ", pass->pw_name);
            else printf("? ");
            
            errno = 0;
            struct group* gr = getgrgid(statbuf.st_gid);    // memory loss
            if (!errno)
                printf("%s ", gr->gr_name);
            else printf("? ");
        } 
        else
            printf("%d %d ", statbuf.st_uid, statbuf.st_gid);

        printf("%6lu ", statbuf.st_size);

        char time_str[15] = {0};
        if (!strftime(time_str, 15, "%b %d %H:%M", gmtime(&statbuf.st_mtime)))
            printf("?");
        else
            printf("%s ", time_str);

        printf("%s \n", name);
    } 
    else
        printf("%s  ", name);

    free (full_dir);

    return 0;
}

//-----------------------------------------------------------------------------------------------------

int ls_func(char* dir_name, char option) {

    DIR* cur_dir = opendir(dir_name);
    if (cur_dir == NULL) {
    
        perror("uls (dir error)");
        return -1;
    }
    if (catch_bit (option, 4)) {
        
        int len = strlen (dir_name);
        if (dir_name[strlen(dir_name) - 1] == '/') dir_name[len - 1] = 0;
        
        printf("%s:\n", dir_name);

        if (len != strlen(dir_name)) dir_name[len - 1] = '/';
    }

    struct dirent* file;
    int status = 0;
    
    while(1){
        
        status = read_dir (cur_dir, &file);
        
        if (status) break;

        if (file->d_name[0] != '.' || catch_bit (option, 0)) {
            char *full_dir = (char *) calloc (sizeof (char), (strlen (dir_name) + 2));

            strcat (full_dir, dir_name);
            if (!strcmp (dir_name, ".."))
                strcat (full_dir, "/");

            print_file (full_dir, file->d_name, option);
            free (full_dir);
        }
    }

    printf("\n");

    if (catch_bit (option, 4)) {
        
        printf("\n");
        rewinddir(cur_dir);

        while(1) {
            status = read_dir(cur_dir, &file);
            if (status) break;

            if (file->d_type == DT_DIR) {
                
                if (!(!strcmp(file->d_name, ".") || !strcmp(file->d_name, ".."))) {
                
                    if (file->d_name[0] != '.' || catch_bit (option, 0)) {
                        
                        size_t name_len = strlen(dir_name) + strlen(file->d_name) + 3;
                        char *full_dir = (char *) malloc (sizeof (char) * name_len);
                        full_dir[0] = 0;

                        strcat(full_dir, dir_name);
                        if (dir_name[strlen(dir_name) - 1] != '/')
                            strcat(full_dir, "/");
                        else --name_len;
                        strcat(full_dir, file->d_name);

                        full_dir[name_len - 2] = '/';
                        full_dir[name_len - 1] = 0;
                        ls_func(full_dir, option);

                        free (full_dir);
                    }
                }
            }
        }
    }
    closedir (cur_dir);
    return status;
}

//-----------------------------------------------------------------------------------------------------

int read_dir(DIR* dir, struct dirent** file) {

    *file = readdir(dir);
    if (*file == NULL) {
        if (errno)
            perror("file error");
        return 1;
    }
    return 0;
}