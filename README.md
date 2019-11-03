# Unix_Shell
This project consists of designing a C program to serve as a shell interface that accepts user commands and then executes each command in a separate process. 


# How to use.

1. Download the file and in your terminal navigate the the file and type in the command make prog to build.
2. Then run the program type in ./prog the terminal should show a " osh>" promting the user to enter a command.


# Simple Commands
osh>ls > out.txt
the output from the ls command will be redirected to the file out.txt. Similarly, input can be redirected as well. For example, if the user enters

osh>sort < in.txt
the file in.txt will serve as input to the sort command.

dup2(fd, STDOUT_FILENO);
duplicates fd to standard output (the terminal). This means that any writes to standard output will in fact be sent to the out.txt file.

osh>ls -l | less
has the output of the command ls −l serve as the input to the less command. Both the ls and less commands will run as separate processes and will communicate using the UNIX pipe() function




