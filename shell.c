#include "shell.h"


void check_memory(void* block)
{
    if(block == NULL)
    {
        fprintf(stderr, "Error of memory allocation!");
        exit(EXIT_FAILURE);
    }
}

buffer_t* create_buffer()
{
    buffer_t* buffer = malloc(sizeof(buffer_t));

    check_memory(buffer);

    buffer->buff_size = LSH_TOK_BUFSIZE; // размер буфера 
	buffer->str = malloc(LSH_TOK_BUFSIZE);

    check_memory(buffer->str);

    memset(buffer->str, 0, buffer->buff_size);

    buffer->program = NULL;
    buffer->args = NULL;

    return buffer;
}

void lsh_loop()
{
    buffer_t* buffer = create_buffer();
    int status; // статус исполнения
    init_directory_system();

    printf("Command, my lord!\n");
    do
    {
        GetCurrentDirectory(sizeof(buffer->directory), buffer->directory);
        printf("%s> ", buffer->directory); 
        buffer = lsh_read_line(buffer); // чтение всей строки
        lsh_split_line(buffer); // парсинг аргументов и программы
        status = lsh_execute(buffer); // выполнение

        free(buffer->str);
        free(buffer); // очистка 
    } while (status);  
}

buffer_t* lsh_read_line(buffer_t* buffer)
{
	int i = 0; // итератор 
	int c;

	while (1)
	{
		c = getchar(); // считывания символа 

		if (c == EOF || c == '\n') // проверка на конец строки 
		{
			buffer->str[i] = '\0';
			return buffer;; // возвращаем введенную строку 
		}
		else if (c == ' ') // разделение на токены 
		{
                buffer->str[i] = ' ';
			    buffer->str[i + 1] = '\\';
			    buffer->str[i + 2] = ' ';
			    i += 2;
		}
		else 
		{
			buffer->str[i] = c; // запись символа в буфер
		}

		i++;

		if (i >= buffer->buff_size) // проверка на границы буфера
		{
			buffer->str = realloc(buffer->str, buffer->buff_size + LSH_TOK_BUFSIZE); // перераспределяем буфер
			check_memory(buffer->str);
            memset(buffer->str + buffer->buff_size, 0, LSH_TOK_BUFSIZE);
            buffer->buff_size += LSH_TOK_BUFSIZE; // +64 байта к текущему буферу
		}
	}
}

void lsh_split_line(buffer_t* buffer)
{
	buffer->program = strtok(buffer->str, LSH_TOK_DELIM); // парсинг первой(главной) команды
	buffer->args = buffer->str + strlen(buffer->program) + 1; // парсинг аргументов
}

/*
    Данный shell работает только на "форточках", 
    так как используется функция только для винды 
*/

int lsh_launch(buffer_t* buffer)
{
    STARTUPINFO si; // структура STARTUPINFO используется для задания дополнительных параметров запуска процесса, например настройка дескрипторов ввода/вывода, настройка ширины окна и etc.

    PROCESS_INFORMATION pi; // Структура PROCESS_INFORMATION содержит информацию о новом созданном процесс
    

    ZeroMemory(&si, sizeof(si)); // заполнение блока памяти нулями(очистка) 
    si.cb = sizeof(si); // размер структуры
    ZeroMemory(&pi, sizeof(pi)); // заполнение блока памяти нулями(очистка) 
    
    printf("%s ", buffer->args);
     if( !CreateProcess(
        buffer->program,    // Имя исполняемого модуля
        buffer->args,       // Аргументы
        NULL,               // Здесь определяются атрибуты защиты для нового приложения. Если указать NULL то система сделает это по умолчанию.
        NULL,               // Здесь определяются атрибуты защиты для первого потока созданного приложением. NULL опять приводит к установке по умолчанию.
        FALSE,              // Флаг наследования от процесса производящего запуск. Здесь наследуются дескрипторы
        0,                  // Флаг способа создание процесса
        NULL,               // Указывает на блок среды. Если NULL, то будет использован блок среды родительского процесса.
        NULL,               // Указывает текущий диск и каталог. Если NULL то будет использован диск и каталог процесса родителя.
        &si,                // Указатель на структуру STARTUPINFO
        &pi )               // Указатель на структуру PROCESS_INFORMATION
    ) 
    {
        printf("Something went wrong (%d)", GetLastError);  
        exit(EXIT_FAILURE);              
    }

     WaitForSingleObject(pi.hProcess, INFINITE); // останавливает выполнения программы до тех пор пока созданный процесс не завершится
     // TODO : переделать эту часть кода, чтобы shell не ждал завершения созданного процесса

     CloseHandle(pi.hProcess);
     CloseHandle(pi.hThread);
}

int lsh_num_builtins()
{
    return sizeof(builtin_str) / sizeof(char *);
}

int lsh_cd(buffer_t* buffer)
{
    if(*(buffer->args) == 0)
    {
        fprintf(stderr, "No arguments\n");        
    }
    else 
    {
        if(!SetCurrentDirectory(buffer->args + 2))
        {
            printf("This directory does not exist\n");
        }    
    }

    return 1;
}


int lsh_help(buffer_t* buffer)
{
    printf("\n\t This shell was written by alex.jpeg;\n"
            "\t Date of writing 06.10.2019;\n"
            );

    for(int i = 0; i < LSH_NUM_BUILTINS; i++)
    {
        printf("\t %s\n", builtin_str[i]);
    }

    printf("\t Use those comand\n");
    return 1;
}

int lsh_exit(buffer_t* buffer)
{
    exit(EXIT_SUCCESS);
}

int lsh_clear(buffer_t* buffer)
{
    clrscr();
    return 1;
}


void init_directory_system()
{
    char* dir_sys = getenv("PATH");
    buffer_directory_system[0] = strtok(dir_sys, ";");

    for(int i = 1; i < 30; i++)
    {
        buffer_directory_system[i] = strtok(NULL, ";");

        if(buffer_directory_system[i] == NULL) break;
    }
    
}

int check_files(buffer_t* buffer)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hf;
    char buffer_directory[64];


    strcpy(buffer_directory, buffer->directory);
    strcpy(buffer_directory + strlen(buffer->directory), "\\*");

    hf = FindFirstFile(buffer_directory, &FindFileData);

    if(INVALID_HANDLE_VALUE != hf)
    {
        do
        {
            if(strcmp(buffer->program, FindFileData.cFileName) == 0)
            {
                return 1;
            }

        } while (FindNextFile(hf, &FindFileData) != NULL);
 
        FindClose(hf);
    }  

    int size = 0;
    for(int i = 0; i < 30; i++)
    {
        if(buffer_directory_system[i] == NULL) break;

        strcpy(buffer_directory, buffer_directory_system[i]);
        size = strlen(buffer_directory_system[i]);
        strcpy(buffer_directory + size, "\\*");

        hf = FindFirstFile(buffer_directory, &FindFileData);
        
        do
        {
            if(strcmp(buffer->program, FindFileData.cFileName) == 0)
            {
                char buf_par[128];
                strcpy(buf_par, buffer->args);
                strcpy(buffer->program, buffer_directory_system[i]);
                strcpy(buffer->program + size ,"\\");
                strcpy(buffer->program + size + 1, FindFileData.cFileName);
                strcpy(buffer->program + strlen(buffer->program), buf_par);
                printf("%s\n", buffer->args);
                FindClose(hf);
                return 1;
            }

        } while (FindNextFile(hf, &FindFileData) != NULL);
    }

    
    FindClose(hf);
    return 0;
}

void ls_print_file(buffer_t* buffer) 
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hf;
    char buffer_directory[64];


    strcpy(buffer_directory, buffer->directory);
    strcpy(buffer_directory + strlen(buffer->directory), "\\*");

    hf = FindFirstFile(buffer_directory, &FindFileData);

    if(INVALID_HANDLE_VALUE != hf)
    {
        do
        {
            printf("%s\n", FindFileData.cFileName);

        } while (FindNextFile(hf, &FindFileData) != NULL);
 
        FindClose(hf);
    }  
}

int lsh_ls(buffer_t* buffer)
{
    ls_print_file(buffer);
    return 1;
}

int lsh_execute(buffer_t* buffer)
{
    for(int i = 0; i < lsh_num_builtins(); i++)
    {
        if(strcmp(buffer->program, builtin_str[i]) == 0)
        {
            return (*array_of_fun[i])(buffer);
        }
    }

    if(!check_files(buffer))
    {
        printf("This program or comand \"%s\" does not exist. Check the spelling\n", buffer->program);
        return 1;
    }

    return lsh_launch(buffer);
}

void clrscr()
{
    HANDLE sys_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info;
    COORD topleft = { 0 };
    DWORD size, written;
    if (!GetConsoleScreenBufferInfo(sys_stdout, &info))
        return;
    size = info.dwSize.X * info.dwSize.Y;
    /* Overwrite the visible buffer with blank spaces */
    FillConsoleOutputCharacter(sys_stdout, ' ', size, topleft, &written);
    /* If the first call succeeded, this one should too */
    GetConsoleScreenBufferInfo(sys_stdout, &info);
    /*
        Fix the character attributes (color, etc...) for the "whitespace"
        if they weren't set to defaults. Otherwise they would be lost.
        Finally, reset the cursor position.
    */
    FillConsoleOutputAttribute(sys_stdout, info.wAttributes, size, topleft, &written);
    SetConsoleCursorPosition(sys_stdout, topleft);
}