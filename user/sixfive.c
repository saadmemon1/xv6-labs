#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// Separator characters
char *seps = " -\r\t\n./,";

int is_sep(char c) {
  return strchr(seps, c) != 0;
}

int main(int argc, char *argv[]) {
  if(argc < 2){
    fprintf(2, "\nError! Correct usage: sixfive <file.txt>\n");
    exit(1);
  }

  for(int i = 1; i < argc; i++){
    int fd = open(argv[i], O_RDONLY);
    if(fd < 0){
      fprintf(2, "\nError! cannot open %s\n", argv[i]);
      continue;
    }

    char buf[1];
    int nlen = 0;
    char numbuf[64];

    while(read(fd, buf, 1)){
      char a = buf[0];
      if(!is_sep(a) && (nlen > 0 || (a >= '0' && a <= '9'))){
        if(nlen < sizeof(numbuf)-1){
          numbuf[nlen++] = a;
        }
      } else {
        if(nlen > 0){
          numbuf[nlen] = '\0';
          int finalValue = atoi(numbuf);
          if(finalValue % 5 == 0 || finalValue % 6 == 0){
            printf("%d\n", finalValue);
          }
          nlen = 0;
        }
      }
    }

    if(nlen > 0){
      numbuf[nlen] = '\0';
      int finalValue = atoi(numbuf);
      if(finalValue % 5 == 0 || finalValue % 6 == 0){
        printf("%d\n", finalValue);
      }
    }

    close(fd);
  }

  exit(0);
}

