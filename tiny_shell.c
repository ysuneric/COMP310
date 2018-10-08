//Eric Sun 260673824
//pls no copy
#define SSIZE 8192
#define _GNU_SOURCE
#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <wordexp.h>
#include <time.h>
#include <sched.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

//NOTE: TIMER IS CURRENTLY COMMENTED OUT
int direct; //global variable for pipe execution
char *fp; //global pointer for fifo pipe name

int clone_functions(void *arg)
{    
    int status = 0;
    char *input = (char *) malloc(sizeof(arg)); //malloc necessary sizes
    input  = (char *) arg; //cast argument into char *

    if((execl("/bin/sh", "/bin/sh", "-c", input, 0)) == -1)
    {
        printf("execution error");
        _exit(-1);
    }
    
    printf("clone execution failed");
    _exit(-1);
}

int my_system(char *line)
{
    int status = 0; //status for process management

    #ifdef FORK //implement with fork
        pid_t pid = fork(); //fork
        if(pid == 0) //child process
        {
            if((execl("/bin/sh", "/bin/sh", "-c", line, 0)) == -1)
            {
                printf("execution error");
                _exit(-1);
            }
            printf("fork execution failed");
            _exit(-1); //command failed
        }
        if(waitpid(-1, &status, 0) == -1) //wait for child process to finish
        {
            status = -1;
        }
        return status; //return to main loop

    #elif VFORK //implement with vfork
        pid_t pid = vfork(); //fork
        if (pid ==0)
        {
            if((execl("/bin/sh", "/bin/sh", "-c", line, 0)) == -1)
            {
                printf("execution error");
                _exit(-1);
            }
            printf("vfork execution failed");
            _exit(-1);//command failed
        }
        if(waitpid(-1, &status, 0) == -1) //wait for child process to finish
        {
            status = -1;
        } 
        return status;

    #elif CLONE //implement with clone
        void *stack = malloc(SSIZE); //initialize stack
        if(stack == NULL) //if stack allocation fails
        {
            printf("stack allocation failed");
            exit(-1);
        }
        void *stackTop = stack + SSIZE; //set pointer to top of stack

        //call clone command, return status value
        pid_t pid = clone(clone_functions, stackTop, CLONE_VM| CLONE_FILES| CLONE_FS| SIGCHLD| CLONE_VFORK, line);

	    if(waitpid(-1, &status, 0) == -1) //wait for child process to finish
	    {
            status = -1;
        }

        free(stack); //free stack memory
        return status; //return to main loop

    #elif PIPE //implement with FIFO pipe

        int fd; //declare pipe path and make fifo pipe
        char *input = (char *) malloc(512);
        input = "/tmp/";
        strcat(input, fp);

        pid_t pid;
        pid = fork();
        if (pid == 0) //child function
        {
            if(direct == 1) //write
            {
                close(1); //close stdout
                if ((fd = open(input, O_WRONLY)) < 0) //open new fifo pipe in stdout location
                {
                    printf("open pipe error"); //if pipe doesn't exist, or other error
                    _exit(-1);
                }
                if((execl("/bin/sh", "/bin/sh", "-c", line, 0)) == -1) //execute command, will pipe to second instance
                {
                    printf("execution error");
                    _exit(-1);
                }

            }
            else //read
            {
                close(0); //close stdin
                if ((fd = open(input, O_RDONLY)) < 0) //open fifo pipe in stdin location
                {
                    printf("open pipe error");
                    _exit(-1);
                }
                if((execl("/bin/sh", "/bin/sh", "-c", line, 0)) == -1) //execute command with new input
                {
                    printf("execution error");
                    _exit(-1);
                }
            }
            printf("pipe execution failure"); //other execution failure
            _exit(-1);
        }
	    if(waitpid(-1, &status, 0) == -1) //wait for child process to finish
	    {
            status = -1;
        }
        return status;

    #else
        status = system(line);//implement regular system call if no compiler flag is included
        return status;

    #endif
        printf("system call error");
        exit(-1); //some other kind of error 
}

int main(int argc, char **argv)
{
    //clock_t start = clock();

    #ifdef PIPE
        if(argc < 3) //check that arguments are valid
        {
            printf("no fifo pipe and/or pipe direction defined");
            exit(-1);
        }
        fp = argv[1];
        printf("%s\n", fp);
        if(strcmp(argv[2], "1") == 0) //check if we want this shell to read or write to pipe
        {
            direct = 1;
        }
        else if(strcmp(argv[2], "0") == 0)
        {
            direct = 0;
        }
        else //error if no valid argument given
        {
            printf("pipe must be defined as boolean true(1) = write & false(0) = read");
            exit(-1);
        }
    #endif

    while (1)
    {
        //figure out how to quit after reaching the end of a text
        char input[512]; //getting lines as string array

        if(fgets(input, 512, stdin)) //get first command
        {
            if (strlen(input) > 2) //make sure argument is valid
            {
                int quit = strcmp(input, "exit\n");
                if (quit == 0) //quit if exit is called
                {
                    //clock_t end = clock();
                    //double timer = (double)(end - start)/CLOCKS_PER_SEC;
                    //printf("%f\n", timer);
                    exit(0);
                }
                else
                {
                    strtok(input, "\n");
                    if((my_system(input)) == -1)
                    {
                        printf("error occured\n");
                    }
                }
            }
            else
            {
                printf("invalid command\n"); //COMMAND INVALID
            }   
        }
        else
        {
            //clock_t end = clock();
            //double timer = (double)(end - start)/CLOCKS_PER_SEC;
            //printf("%f\n", timer);
            exit(0); //to prevent infinite loops
        } 
    }
}