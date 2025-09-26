#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"

// Imported from grep.c for regex matching

int match(char*, char*);
int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}




void find(char *path, char *target, char **exec_args) {
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
            fprintf(2, "\nError! find: cannot stat %s\n", buf);
            continue;
        }
        if(st.type == T_FILE){
            if(match(target, de.name)){
                if(exec_args == 0) {
                    printf("%s\n", buf);
                } else {
                    int pid = fork();
                    if(pid == 0){
                        char *args[MAXARG];
                        int i = 0;
                        while(exec_args[i] != 0){
                            args[i] = exec_args[i];
                            i++;
                        }
                        args[i] = buf;
                        args[i+1] = 0;
                        exec(args[0], args);
                        fprintf(2, "\nError! find: exec %s failed\n", args[0]);
                        exit(1);
                    } else {
                        wait(0);
                    }
                }
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
            find(buf2, target, exec_args);
        }
    }
}

int main(int argc, char *argv[]) {
    if(argc < 3){
        fprintf(2, "\nError! Correct usage: find <path> <filename> <optional flags e.g. -exec command ...>\n");
        exit(1);
    }

    // Check for -exec
    int exec_index = -1;
    for(int i = 3; i < argc; i++){
        if(strcmp(argv[i], "-exec") == 0){
            exec_index = i;
            break;
        }
    }

    if(exec_index == -1){
        // No -exec
        find(argv[1], argv[2], 0);
    } else {
        // -exec found
        char *exec_args[MAXARG];
        int n = 0;
        for(int i = exec_index + 1; i < argc; i++){
            exec_args[n++] = argv[i];
        }
        if (exec_args[n-1] == 0) {
            fprintf(2, "\nError! find: -exec requires a command\n");
            exit(1);
        }
        exec_args[n] = 0;

        find(argv[1], argv[2], exec_args);
    }
    exit(0);
}