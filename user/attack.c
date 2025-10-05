#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{

  int SIZE = 128*4096;
  char *memory = sbrk(SIZE);
  char stringToBeMatched[] = "This may help.";
  int stringLen = 14;

  for(int i = 0; i < SIZE - stringLen; i ++) {
    int match = 1;
    for(int j = 0; j < stringLen; j++) {
      if(memory[i + j] != stringToBeMatched[j]) {
        match = 0;
        break;
      }
    }

    if(match) {
      char *secret = &memory[i + 16];
  // Check if the secret is non-empty and printable
      if(secret[0] && secret[0] != '\0' && ((secret[0] >= '0' && secret[0] <= '9') || (secret[0] >= 'A' && secret[0] <= 'Z') || (secret[0] >= 'a' && secret[0] <= 'z'))) {
        printf("%s\n", secret);
      }
    }

  }
  exit(1);
}
