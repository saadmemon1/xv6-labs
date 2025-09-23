#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

void find(char *path, char *target) {
    // Open the directory at 'path'
    int fd;
    struct stat st;

    if((fd = open(path, O_RDONLY)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    if(st.type != T_DIR){
        fprintf(2, "find: %s is not a directory\n", path);
        close(fd);
        return;
    }
    char buf[512], *p;
    struct dirent de;

    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            fprintf(2, "find: cannot stat %s\n", buf);
            continue;
        }
        if(st.type == T_FILE){
            // For each entry in the directory:
            // If entry is a file:
            // If entry name matches 'target', print full path
            if(strcmp(de.name, target) == 0){
                printf("%s\n", buf);
            }
        } else if(st.type == T_DIR && strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0){
            // If entry is a directory and not "." or "..":
            // Build new path: path + "/" + entry name
            char buf2[512];
            int len = strlen(path);
            memmove(buf2, path, len);
            buf2[len] = '/';
            memmove(buf2 + len + 1, de.name, DIRSIZ);
            buf2[len + 1 + DIRSIZ] = 0;
            // Recursively call find(new path, target)
            find(buf2, target);
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc != 3){
        fprintf(2, "\nError! Correct usage: find <path> <filename>\n");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}