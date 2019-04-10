
// C Program to design a shell in Linux
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<readline/readline.h>
#include<readline/history.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")

// Greeting shell during startup
//
void init_shell()
{
    clear();
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****MY SHELL****");
    printf("\n\n\t-USE AT YOUR OWN RISK-");
    printf("\n\n\n\n*******************"
        "***********************");
    char* username = getenv("USER");
    printf("\n\n\nUSER is: @%s", username);
    printf("\n");
    sleep(1);
    clear();
}

char *replace_str(char *str, char *orig, char *rep, int start)
{
  static char temp[4096];
  static char buffer[4096];
  char *p;

  strcpy(temp, str + start);

  if(!(p = strstr(temp, orig)))  // Is 'orig' even in 'temp'?
    return temp;

  strncpy(buffer, temp, p-temp); // Copy characters from 'temp' start to 'orig' str
  buffer[p-temp] = '\0';

  sprintf(buffer + (p - temp), "%s%s", rep, p + strlen(orig));
  sprintf(str + start, "%s", buffer);

  return str;
}

// Function to take input
int takeInput(char* str)
{
    char* buf;

    buf = readline(" [ ");
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}

// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char* username = getenv("USER");
    char dir[1024];
    sprintf(dir, "/home/%s", username);
    char* d = replace_str(cwd, dir, "~", 0);
    printf("\n%s", d);
}

// Function where the system command is executed
void execArgs(char** parsed)
{
    // Forking a child
    pid_t pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        wait(NULL);
        return;
    }
}

// Function where the piped system commands is executed
void execArgsPiped(char** parsed, char** parsedpipe)
{
    // 0 is read end, 1 is write end
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) < 0) {
        printf("\nPipe could not be initialized");
        return;
    }
    p1 = fork();
    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0) {
        // Child 1 executing..
        // It only needs to write at the write end
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        // Parent executing
        p2 = fork();

        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }

        // Child 2 executing..
        // It only needs to read at the read end
        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {
            // parent executing, waiting for two children
            wait(NULL);
            wait(NULL);
        }
    }
}

// Help command builtin
void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\nCopyright @ Suprotik Dey"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"
        "\n>improper space handling");

    return;
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
    int NoOfOwnCmds = 5, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* username;
    char* arg = parsed[1];

    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "manual";
    ListOfOwnCmds[3] = "memoryinfo";
	ListOfOwnCmds[4] = "systeminfo";

    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }

    switch (switchOwnArg) {
    case 1:
        write_history("history.txt");
        printf("\nGoodbye\n");
        if(arg != NULL){
            int x;
            sscanf(arg, "%d", &x);
            printf("%d",x);
            exit(x);
        }else{
            exit(0);
        }
    case 2:
        chdir(arg);
        return 1;
    case 3:
        openHelp();
        return 1;
    case 4:
        if(arg != NULL){

        	if(strcmp(arg, "-s")==0){
  				system("free  | grep ^Mem | tr -s ' ' | cut -d ' ' -f 5");
  				return 1;
        	}else if(strcmp(arg, "-u")==0){
            	system("free  | grep ^Mem | tr -s ' ' | cut -d ' ' -f 3");
            	return 1;
        	}else if(strcmp(arg, "-t")==0){
				system("free  | grep ^Mem | tr -s ' ' | cut -d ' ' -f 2");
				return 1;
        	}
		}else{
			printf("Could not execute command. Please add -s, -u, or -t argument");
			return 1;
		}
	case 5:
		if(arg != NULL){

			if(strcmp(arg, "-k")==0){
				system("uname -r");
                return 1;
			}else if(strcmp(arg, "-h")==0){
                system("hostname");
                return 1;
            }else if(strcmp(arg, "-o")==0){
                system("hostnamectl | grep Operating | tr -s ' ' | cut -d ' ' -f 4,5,6,7,8,9");
                return 1;
            }else if(strcmp(arg, "--help")==0){
                printf("\nUsage: systeminfo [OPTION]"
                        "\nDisplay information about the current system"
                        "\nExample: systeminto -h"
                        "\n"
                        "\nOptions:"
                        "\n-k,      Prints kernal version to the terminal"
                        "\n-h,      Prints hostname to the terminal"
                        "\n-o,      prints operating system to the terminal"
                        "\n--help,      displays this menu"
                        "\n");
                return 1;
            }
        }else{
                printf("\nUsage: systeminfo [OPTION]"
                        "\nDisplay information about the current system"
                        "\nExample: systeminto -h"
                        "\n"
                        "\nOptions:"
                        "\n-k,      Prints kernal version to the terminal"
                        "\n-h,      Prints hostname to the terminal"
                        "\n-o,      prints operating system to the terminal"
                        "\n--help,      displays this menu"
                        "\n");
                return 1;
        }


    default:
        break;
    }

    return 0;
}

// function for finding pipe
int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] = strsep(&str, "|");
        if (strpiped[i] == NULL)
            break;
    }

    if (strpiped[1] == NULL)
        return 0; // returns zero if no pipe is found.
    else {
        return 1;
    }
}

// function for parsing command words
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
    read_history("history.txt");
    //init_shell();

    while (1) {
        // print shell line
        printDir();
        write_history("history.txt");
        // take input
        if (takeInput(inputString))
            continue;
        // process
        execFlag = processString(inputString,
        parsedArgs, parsedArgsPiped);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
        // 2 if it is including a pipe.

        // execute
        if (execFlag == 1)
            execArgs(parsedArgs);

        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
    }
    return 0;
}
