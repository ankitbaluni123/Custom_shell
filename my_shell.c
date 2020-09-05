#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<unistd.h> 
#include<sys/types.h> 
#include<sys/wait.h> 
#include<readline/readline.h> 
#include<readline/history.h> 
#define MAXCOM 1000
#define MAXLIST 100

#define clear() printf("\033[H\033[J") 

void init_shell() 
{ 
    clear(); 
    char* username = getenv("USER"); 
    printf("\n\n\nUSER is: @%s", username); 
    printf("\n"); 
    sleep(0.5); 
    clear(); 
} 


int takeInput(char* str) 
{ 
    char* buf; 
    buf = readline("\n>>> "); 
    if (strlen(buf) != 0) { 
        add_history(buf); 
        strcpy(str, buf); 
        return 0; 
    } else { 
        return 1; 
    } 
} 


void printDir() 
{ 
    char cwd[1024]; 
    getcwd(cwd, sizeof(cwd)); 
    printf("\nDir: %s", cwd); 
} 


void execArgs(char** parsed,char* str) 
{ 
    pid_t pid = fork();  
    if (pid == -1) { 
        printf("\nFailed"); 
        return; 
    } else if (pid == 0) { 
        if(str!="") { printf("\n Bash command for the %s command is %s\n\n",parsed[0],str); execvp(str, parsed); }
        else execvp(parsed[0],parsed);
        exit(0); 
    } else { 
        wait(NULL);  
        return; 
    } 
} 

void execArgsPiped(char** parsed, char** parsedpipe) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t p1, p2; 
    pipe(pipefd);
    p1 = fork(); 
    if (p1 == 0) { 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
        execvp(parsed[0], parsed);
    } else {  
        p2 = fork(); 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]);
            execvp(parsedpipe[0], parsedpipe);
        } else { 
            wait(NULL); 
            wait(NULL); 
        } 
    } 
} 

// Here add the code you have to use........
int ownCmdHandler(char** parsed) 
{
    // **************************** 
    if(strcmp(parsed[0],"directory")==0){
        execArgs(parsed,"ls"); return 1; }

    //******************************
 
    return 0; 
} 

int parsePipe(char* str, char** strpiped) 
{ 
    int i; 
    for (i = 0; i < 2; i++) { 
        strpiped[i] = strsep(&str, "|"); 
        if (strpiped[i] == NULL) 
            break; 
    } 
  
    if (strpiped[1] == NULL) 
        return 0;
    else { 
        return 1; 
    } 
} 

void parseSpace(char* str, char** parsed) 
{ 
    int i; 
  
    for (i = 0; i < MAXLIST; i++) { 
        parsed[i] = strsep(&str, " "); 
  
        if (parsed[i] == NULL) 
            break; 
        if (strlen(parsed[i]) == 0) 
            i--; 
    } 
} 
  
int processString(char* str, char** parsed, char** parsedpipe) 
{ 
  
    char* strpiped[2]; 
    int piped = 0; 
  
    piped = parsePipe(str, strpiped); 
  
    if (piped) { 
        parseSpace(strpiped[0], parsed); 
        parseSpace(strpiped[1], parsedpipe); 
  
    } else { 
        parseSpace(str, parsed); 
    } 

    if (ownCmdHandler(parsed)) 
        return 0; 
    else
        return 1 + piped; 
} 
  
int main() 
{ 
    char inputString[MAXCOM], *parsedArgs[MAXLIST]; 
    char* parsedArgsPiped[MAXLIST]; 
    int execFlag = 0; 
    init_shell(); 
  
    while (1) { 
        printDir();  
        if (takeInput(inputString)) 
            continue; 
        execFlag = processString(inputString,parsedArgs, parsedArgsPiped); 

        if (execFlag == 1) 
            execArgs(parsedArgs,""); 
  
        if (execFlag == 2) 
            execArgsPiped(parsedArgs, parsedArgsPiped); 
    } 
    return 0; 
} 