#include <stdio.h>
#include <sys/resource.h>

int main() {
    struct rlimit rl;
    if (getrlimit(RLIMIT_STACK, &rl) == 0)  printf("stack size: %lld\n", (long long)rl.rlim_cur);
    if (getrlimit(RLIMIT_NPROC, &rl) == 0)  printf("process limit: %lld\n", (long long)rl.rlim_cur);
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0)  printf("max file descriptors: %lld\n", (long long)rl.rlim_cur);
    return 0;
}
