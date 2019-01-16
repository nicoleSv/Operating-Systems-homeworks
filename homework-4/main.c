#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>

const int READ = 0;
const int WRITE = 1;
const int MAX_CHARS = 1024;
const int MAX_ARGS = 64;

int to_int(char* string)
{
    int length = strlen(string);
    int count = 0;

    for(int i = 0; i < length; i++)
    {
        if(string[i] < '0' || string[i] > '9')
            break;

        count++;
    }

    if(count == length)
        return atoi(string);
    else
        return -1;
}

//reads a line from stdin
//returns 1 on success and 0 if nothing to read
int read_line(char* line)
{
    int position = 0;
    char buffer;

    while(read(READ, &buffer, sizeof(buffer)))
    {
        if(buffer == '\n')
        {
            line[position] = '\0';
            return 1;
        }
        else
            line[position] = buffer;

        position++;
    }

    return 0;
}

char* get_substring(char* string, int start, int end)
{
    int length = end - start;
    char* sub = (char*)malloc(length*sizeof(char));
    int i = 0;

    for(int j = start; j < end; j++)
    {
        if((j == start || j == end-1) && string[j] == ' ')
            continue;
        sub[i] = string[j];
        i++;
    }
    sub[i] = '\0';

    return sub;
}

//splits a string by delimiter into 2d array
char** split_line(char* line, char delimiter)
{
    char** tokens = (char**)malloc(MAX_ARGS*sizeof(char*));
    int i_tokens = 0;
    int start = 0, end = 0;
    int length = strlen(line);

    for(int i = 0; i < length; i++)
    {
        if(line[i] == delimiter)
        {
            end = i;
            tokens[i_tokens] = get_substring(line, start, end);
            i_tokens++;
            start = i + 1;
        }
    }

    tokens[i_tokens] = get_substring(line, start, length);
    tokens[i_tokens+1] = NULL;

    return tokens;
}

//executes single command without pipes
void simple_command(char** command)
{
    pid_t pid = fork();
    if(pid == 0)
    {
        execvp(command[0], command);
        err(1, "Failed to execute %s", command[0]);
    }
    else if(pid > 0)
        wait(NULL);
    else
        err(1, "Failed to fork!");
}

int count_pipes(char* pipeline)
{
    int count = 0;
    int i = 0;

    while(pipeline[i])
    {
        if(pipeline[i] == '|')
            count++;
        i++;
    }

    return count;
}

//splits piped commands into 3d array
char*** split_pipes(char* pipeline)
{
    int total_commands = count_pipes(pipeline) + 1;
    char*** commands = (char***)malloc((total_commands + 1)*sizeof(char**));
    int i_commands = 0;
    char* simple_command;
    int length = strlen(pipeline);
    int start = 0, end = 0;

    for(int i = 0; i < length; i++)
    {
        if(pipeline[i] == '|')
        {
            end = i;

            simple_command = get_substring(pipeline, start, end);
            commands[i_commands] = split_line(simple_command, ' ');

            i_commands++;
            start = i + 1;
        }
    }

    simple_command = get_substring(pipeline, start, length);
    commands[i_commands] = split_line(simple_command, ' ');
    commands[i_commands + 1] = NULL;

    free(simple_command);

    return commands;
}

//executes multiple piped commands
void multiple_pipes(char*** commands)
{
    int pipe_fd[2];
    pid_t pid;
    int fd_in = 0;
    int i = 0;

    while(commands[i])
    {
        if(pipe(pipe_fd) == -1)
            err(1, "Failed to create pipe!");

        pid = fork();
        if(pid == 0)
        {
            if(dup2(fd_in, READ) == -1)
                err(1, "Failed to duplicate!");

            if(commands[i+1])
            {
                if(dup2(pipe_fd[WRITE], WRITE) == -1)
                    err(1, "Failed to duplicate!");
            }

            if(close(pipe_fd[READ]) == -1)
                err(1, "Failed to close pipe end");

            execvp(commands[i][0], commands[i]);
            err(1, "Failed to execute %s", (commands[i][0]));
        }
        else if(pid > 0)
        {
            wait(NULL);

            if(close(pipe_fd[WRITE]) == -1)
                err(1, "Failed to close pipe end!");

            fd_in = pipe_fd[READ];
            i++;
        }
        else
            err(1, "Failed to fork!");
    }
}

//executes a task from stdin
void process_line(char* input)
{
    pid_t pid = fork();
    if(pid == 0)
    {
        char** commands = split_line(input, ';');

        int i = 0;
        while(commands[i])
        {
            if(count_pipes(commands[i]))
            {
                char*** piped_commands = split_pipes(commands[i]);
                multiple_pipes(piped_commands);

                int j = 0, k = 0;
                while(piped_commands[j])
                {
                    while(piped_commands[j][k])
                    {
                        free(piped_commands[j][k]);
                        k++;
                    }

                    free(piped_commands[j]);
                    j++;
                }
                free(piped_commands);
            }
            else
            {
                char** single = split_line(commands[i], ' ');
                simple_command(single);
                
                int j = 0;
                while(single[j])
                {
                    free(single[j]);
                    j++;
                }
                free(single);
            }

            i++;
        }

        int j = 0;
        while(commands[j])
        {
            free(commands[j]);
            j++;
        }
        free(commands);
    }
}

int main(int argc, char* argv[])
{
    if(argc != 2)
        errx(1, "Not enough or too many parameters");

    int max_tasks = to_int(argv[1]);
    if(max_tasks == -1)
        errx(1, "Invalid argument: must be a number");

    char* line = (char*)malloc(MAX_CHARS);

    int all_tasks = 0;
    int current_tasks = 0;

    while(read_line(line))
    {
        process_line(line);
        current_tasks++;

        if(waitpid(0, 0, WNOHANG))
            current_tasks--;

        if(current_tasks == max_tasks)
        {
            wait(NULL);
            current_tasks--;
        }

        all_tasks++;
    }

    for(int i = 0; i < all_tasks; i++)
        wait(NULL);

    free(line);
    return 0;
}
