#include "shell.h"

int main()
{
    setlocale(LC_ALL, "russian");
    lsh_loop();
    return 0;    
}