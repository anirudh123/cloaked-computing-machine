/* 
 * tsh - A tiny shell program with job control
 * @author- Anirudh Sunkineni
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "parser.h"

/* Misc manifest constants */
#define MAXLINE    1024   /* max line size */
#define MAXARGS     128   /* max args on a command line */

/* Global variables */
sigset_t blk;
char prompt[] = "cs551 shell> "; /* Initial shell prompt */
char sbuf[MAXLINE];         /* for composing sprintf messages */

/* End global variables */


/* Function prototypes */

void eval(char *cmdline);
void io_redirection(char *cmdline);
int builtin_cmd(char **argv);

void sigint_handler(int sig);

/* Helper routines */
int parseline(const char *cmdline, char **argv); 

void app_error(char *msg);
typedef void handler_t(int);
handler_t *Signal(int signum, handler_t *handler);

/*
 * main - The shell's main routine 
 */
int main(int argc, char **argv) 
{
    char c;
    char cmdline[MAXLINE];

    /* Redirect stderr to stdout (so that driver will get all output
     * on the pipe connected to stdout) */
    dup2(1, 2);

    /* Install the signal handlers */
    signal(SIGINT,  sigint_handler);   /* ctrl-c */

    /* Execute the shell's read/eval loop */
    while (1) {

	/* Read command line */
	fflush(stdout);
	printf("%s", prompt);
	fflush(stdout);

	if ((fgets(cmdline, MAXLINE, stdin) == NULL) && ferror(stdin)){
	    app_error("fgets error");
    }
    eval(cmdline);
        
	fflush(stdout);
    } 

    exit(0); /* control never reaches here */
}
  
/* 
 * eval - Evaluate the command line that the user has just typed in
 * 
 * If the user has requested a built-in command (quit, jobs, bg or fg)
 * then execute it immediately. Otherwise, fork a child process and
 * run the job in the context of the child. If the job is running in
 * the foreground, wait for it to terminate and then return.  Note:
 * each child process must have a unique process group ID so that our
 * background children don't receive SIGINT (SIGTSTP) from the kernel
 * when we type ctrl-c (ctrl-z) at the keyboard.  
*/
void eval(char *cmdline){
    if(cmdline && cmdline[0] == '\0'){
        return;
    }else if(strcmp(cmdline, "\n")==0){
        return;
    }
    char *argv[MAXARGS];
    int status;
    pid_t pid;
    parseline(cmdline, argv);
    sigemptyset (&blk);
    sigaddset (&blk, SIGCHLD);
    sigaddset (&blk, SIGQUIT);
    sigaddset (&blk, SIGTSTP);
    sigaddset (&blk, SIGTERM);
    sigprocmask (SIG_BLOCK, &blk, NULL);
    if(!builtin_cmd(argv)){
        if(!validate(cmdline)){
            printf("Illegal expression with parentheses\n");
            return;
        }
        struct Queue queues[10]; 
        for(int j=0; j<10; j++){
            Queue queue = createQueue();
            queues[j] = queue;
        }
        parse_cmd(cmdline, queues);
        int num_commands = 0;
        int commands_rem = 0;
        for(int k=0; k<10; k++){
            commands_rem += queues[k].size;
        }
        int old_fds[2];
        int new_fds[2];
        for(int i=9; i>=0; i--){
            char curr_cmd[100];
            memset(curr_cmd, 0, sizeof(curr_cmd));
            strcpy(curr_cmd, queues[i].pop(&queues[i]));
            while(strcmp(curr_cmd, "NOMORE")){
                if(commands_rem>1){
                pipe(new_fds);
                printf("Created a pipe in:%d, out:%d",new_fds[0],new_fds[1]);
                fflush(stdout);
                }
                memset(argv, 0, sizeof(argv));
                parseline(curr_cmd, argv);
                printf("Executing command %s\n", curr_cmd);
                if ((pid = fork ()) == 0)
                {
                     if (num_commands>0){
                         close(0);
                         dup2(old_fds[0], 0);
                         printf("Attached pipe-end:%d to stdin\n", old_fds[0]);
                         close(old_fds[0]);
                         close(old_fds[1]);
                     }
                     if (commands_rem>1){
                         printf("Attached pipe-end:%d to stdout\n", new_fds[1]);
                         close(1);
                         close(new_fds[0]);
                         dup2(new_fds[1], 1);
                         close(new_fds[1]);
                     }
                     dup2(1, 2);
                     if(execvp(argv[0],argv)<0){
                         printf("%s: Command not found.\n", argv[0]);
                         exit(0);
                     }
                }else{
                     if (num_commands>0){
                        close(old_fds[0]);
                        close(old_fds[1]);
                     }
                     if (commands_rem>1){
                        old_fds[0] = new_fds[0];
                        old_fds[1] = new_fds[1];
                     }
                     waitpid(-1, NULL, WUNTRACED);
                     memset(curr_cmd, 0, sizeof(curr_cmd));
                     strcpy(curr_cmd, queues[i].pop(&queues[i]));
                     num_commands++;
                     commands_rem--;
                }            
            }
        }
    }
        return;
    }   

/* 
 * parseline - Parse the command line and build the argv array.
 * 
 * Characters enclosed in single quotes are treated as a single
 * argument.  Return true if the user has requested a BG job, false if
 * the user has requested a FG job.  
 */
int parseline(const char *cmdline, char **argv) 
{
    static char array[MAXLINE]; /* holds local copy of command line */
    char *buf = array;          /* ptr that traverses command line */
    char *delim;                /* points to first space delimiter */
    int argc;                   /* number of args */
    int bg;                     /* background job? */

    strcpy(buf, cmdline);
    if (strlen(buf)==0){
        argv[0] = " ";
        return 1;
    }
    if(buf[strlen(buf)-1]=='\n'){
        buf[strlen(buf)-1] = ' ';  /* replace trailing '\n' with space */
    }
    else{
        char c = ' ';
        strcat(buf,  &c);
    }
    while (*buf && (*buf == ' ')) /* ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    if (*buf == '\'') {
	buf++;
	delim = strchr(buf, '\'');
    }
    else {
	delim = strchr(buf, ' ');
    }

    while (delim) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* ignore spaces */
	       buf++;

	if (*buf == '\'') {
	    buf++;
	    delim = strchr(buf, '\'');
	}
	else {
	    delim = strchr(buf, ' ');
	}
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* ignore blank line */
	return 1;
    
    return 1;
}
/* 
 * builtin_cmd - If the user has typed a built-in command then execute
 *    it immediately.  
 */
int builtin_cmd(char **argv) {
    if (strcmp(argv[0],"exit")==0){
        exit(0);
    }
    else if (strcmp(argv[0]," ")==0){
        return 1;
    }
    else if (strcmp(argv[0],"")==0){
        return 1;
    }
    return 0; /* not a builtin command */
}


/*****************
 * Signal handlers
 *****************/

/* 
 * sigint_handler - The kernel sends a SIGINT to the shell whenver the
 *    user types ctrl-c at the keyboard.  
 */
void sigint_handler(int sig) 
{
    char c[100];
    printf("\nctrl+c Are you sure you want to exit? (Y/N) :");
    gets(c);
    if (c[0]=='Y'){
        exit(0);
    }
	printf("\n%s", prompt);
	fflush(stdout);
    return;
}


/*********************
 * End signal handlers
 *********************/


/***********************
 * Other helper routines
 ***********************/

/*
 * app_error - application-style error routine
 */
void app_error(char *msg)
{
    fprintf(stdout, "%s\n", msg);
    exit(1);
}

/*
 * Signal - wrapper for the sigaction function
 */
handler_t *Signal(int signum, handler_t *handler) 
{
    struct sigaction action, old_action;

    action.sa_handler = handler;  
    sigemptyset(&action.sa_mask); /* block sigs of type being handled */
    action.sa_flags = SA_RESTART; /* restart syscalls if possible */

    if (sigaction(signum, &action, &old_action) < 0)
    fprintf(stdout, "%s: %s\n", "Signal error", strerror(errno));
    exit(1);
    return (old_action.sa_handler);
}
