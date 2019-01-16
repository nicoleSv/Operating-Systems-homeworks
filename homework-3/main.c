#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>

//finds the next word in dictionary
int find_word(int fd, int position)
{
    char c;
    int count=0;
    lseek(fd, position, SEEK_SET);

    while(read(fd,&c,1) > 0)
    {
        count++;
        if(c == '\0')
            break;
    }
    return position+count;
}

//finds the length of word or its translation, depending on the delimiter
int str_length(int fd, int position, char delimiter)
{
    char c;
    int length=0;
    lseek(fd, position, SEEK_SET);
   
    while(read(fd,&c,1) > 0)
    {
        length++;
        if(c == delimiter)
            break;
    }
    return length;
}

void free_resources(int fd, char* word, char* translation)
{
    if(word)
        free(word);
    
    if(translation)
        free(translation);
    
    if(close(fd) == -1)
        err(4, "Could not close file descriptor");
}

int main(int argc, char* argv[])
{
    if(argc != 3)
        errx(1,"Not enough or too many parameters");
    
    char* filename=argv[1];
    char* word=argv[2];

    const int fd=open(filename, O_RDONLY);

    if(fd == -1)
        err(2, "Could not open %s", filename);

    int file_size=lseek(fd, 0, SEEK_END);
    int left=0, right=file_size-1, middle;

    //checking if the first word in dictionary is actually the needed one
    int pos=find_word(fd, left);
    int fw_length=str_length(fd, pos, '\n');
    
    char* first_word=(char*)malloc(fw_length);
    
    lseek(fd, pos, SEEK_SET);
    
    if(read(fd, first_word, fw_length) < 0)
    {
        free_resources(fd, first_word, (char*)NULL);
        err(3, "Could not read %s", filename);
    }
     first_word[fw_length-1]='\0';
        
    if(strcmp(word, first_word) == 0)
    {
        int transl_length=str_length(fd, pos+fw_length, '\0');
        char* transl=(char*)malloc(transl_length);
        
        lseek(fd, pos+fw_length, SEEK_SET);
        
        if(read(fd, transl, transl_length) < 0)
        {
            free_resources(fd, first_word, transl);
            err(3, "Could not read %s", filename);
        }

        if(write(1, transl, transl_length) != transl_length)
        {
            free_resources(fd, first_word, transl);
            err(5, "Could not write to STDOUT");
        }
        
        free_resources(fd, first_word, transl);
        return 0;
    }
    
    //binary searching for the needed word and printing its translation
    while(right-left > 1)
    {
        middle=(left+right)/2;
        
        int correct_position=find_word(fd, middle);
        int length=str_length(fd, correct_position, '\n');
        char* middle_word=(char*)malloc(length);
        
        lseek(fd, correct_position, SEEK_SET);
    
        if(read(fd, middle_word, length) <  0)
        {
            free_resources(fd, middle_word, (char*)NULL);
            err(3,"Could not read %s", filename);
        }
        middle_word[length-1]='\0';

        if(strcmp(word, middle_word) == 0)
        {
            int tr_length=str_length(fd, correct_position+length, '\0');
            char* translation=(char*)malloc(tr_length);
            
            lseek(fd, correct_position+length, SEEK_SET);
            
            if(read(fd, translation, tr_length) < 0)
            {
                free_resources(fd, middle_word, translation);
                err(3, "Could not read %s", filename);
            }

            if(write(1, translation, tr_length) != tr_length)
            {
                free_resources(fd, middle_word, translation);
                err(5, "Could not write to STDOUT");
            }

            free_resources(fd, middle_word, translation);
            return 0;
         }
         else if(strcmp(word, middle_word) < 0)
            right=middle-1;
         else
            left=middle+1;
    }
    
    if(close(fd) == -1)
        err(4, "Could not close file descriptor");
    errx(6, "Could not find '%s' in the dictionary!", word);
}
