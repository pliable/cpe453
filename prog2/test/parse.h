#define MAX_PROCS 10
/* MAX_ARGS is 11 because program (position 0) + ten arguments (position 10) */
#define MAX_ARGS 11
#define ABS_MAX 102
void test_print(char *progs[MAX_PROCS][MAX_ARGS]);
void zero_out(char *progs[MAX_PROCS][MAX_ARGS]);
void parse_cl(char *progs[MAX_PROCS][MAX_ARGS], int argc, char *argv[]);
