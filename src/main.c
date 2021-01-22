#include "ush.h"

int main(int argc, char **argv) {
    Prompt *shell = init_ush(argc, argv);
    mx_ush_loop(shell);
    
    return MX_EXIT_SUCCESS;
}
