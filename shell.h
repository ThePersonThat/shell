#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <locale.h>

//#define LSH_RL_BUFSIZE 1024  размер буффера для старго способа чтения строки

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a\\"
#define LSH_BUFSIZE_DIR 64
#define LSH_NUM_BUILTINS sizeof(builtin_str) / sizeof(char *)



typedef struct 
{
    char* str; // указатель на введную строку
    char* args; // указатель на аргументы
    char* program; // указатель на исполняемый модуль 
    size_t buff_size; // размер буфера
    char directory[LSH_BUFSIZE_DIR] ; // буфер содержащий текущую директорию
}buffer_t;

static char* builtin_str[] = {
    "cd", 
    "help",
    "exit",
    "clear",
    "ls"
};

static char* buffer_directory_system[30] = { NULL }; 


void lsh_loop();
buffer_t* lsh_read_line(buffer_t* buffer);
void lsh_split_line(buffer_t* buffer);
int lsh_launch(buffer_t* buffer);
buffer_t* create_buffer();
int lsh_cd(buffer_t* buffer);
int lsh_help(buffer_t* buffer);
int lsh_exit(buffer_t* buffer);
int lsh_clear(buffer_t* buffer);

void check_memory(void* block);
int lsh_execute(buffer_t* buffer);
int lsh_ls(buffer_t* buffer);
void clrscr();
int check_files(buffer_t* buffer);
void init_directory_system();
void ls_print_file(buffer_t* buffer);

typedef int(*built_fun)(buffer_t* buffer);

static built_fun array_of_fun[] = {
    &lsh_cd,
    &lsh_help,
    &lsh_exit,
    &lsh_clear,
    &lsh_ls,
};