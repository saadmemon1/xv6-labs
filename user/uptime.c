#include "user/user.h"

int main(int argc, char* argv[]) {
    int ticks = uptime();
    printf("System uptime: %d ticks\n", ticks);
    exit(0);
}