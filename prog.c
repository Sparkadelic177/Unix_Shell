#include <stdio.h>
#include <ctype.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define MAX_LINE 80 /* The maximum length command */
#define READ_END 0
#define WRITE_END 1

void sliceArray(char *args[], char commandFormat[], int start, int end, int counter)
{

    free(args[counter]);

    int length = 0;

    //checking for any spaces, allocation just enough space for the string not including space
    for (int i = start; i < end; i++)
    {
        if (commandFormat[i] != ' ')
            length++;
    }

    char token[length];
    int tokenctr = 0;

    for (int i = start; i < end; i++)
    {
        if (commandFormat[i] != ' ') //skipping the space character
        {
            token[tokenctr] = commandFormat[i];
            tokenctr++;
        }
    }
    args[counter] = malloc(length);
    strncpy(args[counter], token, length);
}

//restrcuts the args array when using pipes
void pipeReStruct(char *args[])
{
    int firstCommand = strlen(args[0]);
    int secondCommand = strlen(args[1]);

    //create temp arrays with allocated space
    char reStruct1[firstCommand];
    char reStruct2[secondCommand];

    //copy the first two command in temp arrays
    strcpy(reStruct1, args[0]);
    strcpy(reStruct2, args[1]);

    //clear all of the commands
    for (int i = 0; i < MAX_LINE / 2 + 1; i++)
    {
        args[i] = 0;
    }

    //allocating space the the first two tokens
    args[0] = malloc(firstCommand);
    args[1] = malloc(secondCommand);

    //copying the command to each token
    strcpy(args[0], reStruct1);
    strcpy(args[1], reStruct2);
}

//restructs the args array when using simple redirection
void reStruct(char *args[])
{
    int length = strlen(args[0]);
    char reStruct[length];
    strcpy(reStruct, args[0]);

    for (int i = 0; i < MAX_LINE / 2 + 1; i++)
    {
        args[i] = 0;
    }

    args[0] = malloc(length);
    strcpy(args[0], reStruct);
}

int main(void)

{
    id_t pid, pid_2;
    int should_run = 1;
    const int argsLength = MAX_LINE / 2 + 1;
    char *args[argsLength]; /* command line arguments */
    char str[argsLength];
    char history[argsLength];

    while (should_run)
    {
        //restarting all of the values
        int file,
            length = 0,
            start = 0,
            counter = 0,
            fd[2];

        printf("\nosh>");
        fflush(stdout);

        for (int i = 0; i < argsLength; i++)
        {
            args[i] = 0;
            str[i] = 0;
        }

        length = read(STDIN_FILENO, str, MAX_LINE); //length of the user input

        if (str[0] == '!' && str[1] == '!') //if history command entered
        {

            if (history[0] == 0) //if histroy is empty then send proper message
            {
                printf("No commands in history\n");
            }
            else
            { //else map the most recent command in history to args array
                for (int i = 0; i < strlen(history); i++)
                {
                    if (history[i] == ' ' || history[i] == '\t' || history[i] == '\n' || history[i] == 0) //slice array after each word
                    {
                        sliceArray(args, history, start, i, counter);
                        start = i;
                        ++counter;
                    }
                }
            }
        }
        else
        { //if history command not entered, format args
            for (int i = 0; i < length; i++)
            {
                if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == 0) //slice array after each word
                {
                    sliceArray(args, str, start, i, counter);
                    start = i;
                    counter++;
                }
            }

            for (int i = 0; i < argsLength; i++) //save input history
            {
                history[i] = str[i];
            }
        }

        if (length != 0)
            pid = fork(); //create child process

        if (length < 0)
            printf("there was an error in the command sent");

        if (pid < 0)
        { /* error occurred */
            printf("Fork Failed");
            return 1;
        }

        else if (pid == 0)
        { //if child is created then process what the user has inputted from the console

            if (counter > 2)
            {

                if (*args[1] == '>') //write to the file
                {
                    file = open(args[2], O_WRONLY | O_CREAT | O_TRUNC);
                    dup2(file, STDOUT_FILENO);
                    close(file);
                    reStruct(args);
                    execvp(args[0], args);
                }
                else if (*args[1] == '<') //reading from the file
                {
                    file = open(args[2], O_RDONLY);
                    dup2(file, STDIN_FILENO);
                    close(file);
                    reStruct(args);
                    execvp(args[0], args);
                }
                else if (*args[2] == '|') //if the command requuires to have pipes, inside the first child process
                {

                    //creating the pipe
                    if (pipe(fd) == -1)
                    {
                        printf("Pipe failed");
                        return 1;
                    }

                    //saving the last command before re writing args array
                    char *tempChar[] = {args[3], NULL};

                    //creating the second child
                    pid_2 = fork();

                    if (pid_2 < 0)
                    {
                        printf("someting went wrong with creaing the second child ");
                        return (1);
                    }

                    else if (pid_2 == 0)
                    { //this is the second child processs, which will handle the read operation
                        dup2(fd[READ_END], STDIN_FILENO);
                        close(fd[READ_END]);           //close the write end
                        close(fd[WRITE_END]);          //closing the read thread
                        execvp(tempChar[0], tempChar); //executung the argument
                    }
                    else
                    {                                       //the parent of the second child, execute the the first command, handling the write operation
                        pipeReStruct(args);                 //contructs the args have only the first command
                        dup2(fd[WRITE_END], STDOUT_FILENO); //making a pipe
                        close(fd[WRITE_END]);               //close the write end
                        close(fd[READ_END]);                //closing the read thread
                        execvp(args[0], args);              //executung the argument
                    }
                }
                else
                {
                    if (execvp(args[0], args) == -1)
                    { //command not executed
                        printf("\nError executing command\n");
                    }
                }
            }
            else
            {
                if (execvp(args[0], args) == -1)
                { //command not executed
                    printf("\nError executing command\n");
                }
            }
        }

        else
        {               /* parent process */
            wait(NULL); // blocked from futher execution untill the child process gets done running
        }

        if (str[0] == 48)
            should_run = 0;
        else
            should_run = 1;
    }

    return 0;
}