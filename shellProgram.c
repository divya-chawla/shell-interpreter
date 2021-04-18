/*
    Name - Divya Chawla
    Student ID - #########
    Course - CSC 360
    Assgnment 1 - Implement a SSI
        Part 1: execute basic command
        Part 2: change directory
        Part 3: Run background command
*/


/*
    Header Files
*/
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#define MAX_BUFF 1024



/*
    Linked list for background process
*/
struct bg_pro{
    pid_t pid;
    char command[1024];
    struct bg_pro* next;
}; 
//number of background processes running
int num = 0;


/*
    Tokenize the input arguments by space and store it in an array
*/
int tokenizeBySpace(char *inputLine, char **argv){
    int i = 0;
    int argc = 0;
    argv[0] = strtok(inputLine," ");
    while(argv[i]!= NULL){
        argv[i+1] = strtok(NULL," ");
        i++;
        argc ++;
    }
    return argc; 
}


/*
    Function to change diretory
*/
void changeDirectory(int argc, char **argv){
    char *homeDir = getenv("HOME");

    if(argc == 1 || strcmp(argv[1], "~") == 0) {
        chdir(homeDir);
    }
    else if(strcmp(argv[1], "..") == 0){
        chdir("..");
    }
    else{
        if(chdir(argv[1]) != 0){
            printf("%s", "Sorry! Directory not found\n");
        }
        else{
            //for relative and absolute path
            chdir(argv[1]);
        }
    }
}


/*
    Function to execute bckground command
*/
void exceuteBg(int argc, char **argv, struct bg_pro *root){
    pid_t c;
    c = fork();
    char cmd[MAX_BUFF];
    int i = 0;
    //concatenate the command passed as background process into a string
    for(i = 0; i < argc; i++){
        strcat(cmd, argv[i]);
        if(i != argc-1){
            strcat(cmd, " ");
        }
    }

    if(c == 0){
        //under child process
        if(execvp(argv[0], argv) < 0){
            printf("Sorry! Could not find the background command");
            exit(1);
        }
        else{
            execvp(argv[0], argv);
        }
    }
    else{
        if(num == 0){
            root->pid = c;
            strcpy(root->command, cmd);
            root->next = NULL;
            num++;
            strcpy(cmd,"");
        }
        else{
            struct bg_pro *newProcess = (struct bg_pro*)malloc(sizeof(struct bg_pro));
            struct bg_pro *curPro = (struct bg_pro*)malloc(sizeof(struct bg_pro));

            curPro = root;
            while(curPro->next != NULL){
                curPro = curPro->next;
            }
            // adding new process to the linked list
            newProcess->pid = c;
            strcpy(newProcess->command, cmd);
            newProcess->next = NULL;
            curPro->next = newProcess;
            num++;
            strcpy(cmd,"");
        }
    }
}


/*
    Function to check and print when background job is terminated
*/
void bgTerminate(struct bg_pro *root){
    if(num > 0){
        pid_t ter = waitpid(0, NULL, WNOHANG); 
        while (ter > 0){
            if (root->pid == ter){
                printf("%d: %s has terminated \n", root->pid, root->command);
                root = root->next;
                num--;
            }
            else{
                struct bg_pro *curPro = (struct bg_pro*)malloc(sizeof(struct bg_pro));
                curPro = root;
                while(curPro->next->pid != ter){
                    curPro = curPro->next;
                }
                printf("%d: %s has terminated \n", curPro->next->pid, curPro->next->command);
                curPro->next = curPro->next->next;
                num--;
            }
            ter = waitpid(0, NULL, WNOHANG); 
        }
         
    }
         
}


/*
    Function to print all the background processes in linked list
*/
void bgList(struct bg_pro *root){
    struct bg_pro *curPro = (struct bg_pro*)malloc(sizeof(struct bg_pro));
    curPro = root;
    if(num > 0){
        while(curPro != NULL){
            printf("%d: %s \n",curPro->pid, curPro->command);
            curPro = curPro->next;
        }
    }
    else{
        printf("No background processes running at the moment\n");
    }
    printf("Total Background jobs: %d", num);
    printf("\n");
}


/*
    Function to execute basic commands with parameters
*/
void executeInputLine(int argc, char **argv){
    pid_t c;
    c = fork();
    if(c == 0){
        // under child process
        if(execvp(argv[0], argv) < 0){
            printf("Sorry! Could not find the command \n");
            exit(1);
        }
        else{
            execvp(argv[0], argv);
        }
    }
    else{
        waitpid(c, NULL, 0);
    }
}


/*
    Main function
*/
int main(){
    char *username = getlogin();
    char hostname[MAX_BUFF];
    gethostname(hostname, sizeof(hostname));
    char cwd[MAX_BUFF];
    getcwd(cwd, sizeof(cwd));

    struct bg_pro *root = (struct bg_pro*)malloc(sizeof(struct bg_pro));

    char *argv[MAX_BUFF];    
    int i = 0;

    while(1){
        char line[MAX_BUFF];
        // printing SSI prompt
        getcwd(cwd, sizeof(cwd));
        printf("SSI: %s@%s: %s> ",username, hostname, cwd);
        // getting arguments from user. If the argument passed is /n, then replace it with EOF character
        fgets(line, MAX_BUFF, stdin);
        int len = strlen(line)-1;
        if(line[len] == '\n'){ 
            line[len] = '\0';
        }
        // tokenize the input line separated by " "
        int argc = tokenizeBySpace(line, argv);
        //execute user command 
        if(argc > 0){
            if(strcmp(argv[0],"exit") == 0 || strcmp(argv[0],"quit") == 0 || strcmp(argv[0],"q") == 0){
                exit(1);
            }
            else if(strcmp(argv[0], "cd") == 0){
                // change directory for Part 2
                changeDirectory(argc, argv);
            }
            else if(strcmp(argv[0], "bg") == 0){
                //remove bg and execute command for Part 3
                argc --;
                exceuteBg(argc, argv+1, root);

            }
            else if(strcmp(argv[0], "bglist") == 0){
                //print list of all background process running
                bgList(root);
            }
            else{
                // executing for Part 1
                executeInputLine(argc, argv);
            }    
	    //check if any background process is terminated
            bgTerminate(root);
            printf("\n");
        }
    }
    return 0;
} 