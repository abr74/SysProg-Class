#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"



#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#include "dshlib.h"

/**** 
 **** FOR REMOTE SHELL USE YOUR SOLUTION FROM SHELL PART 3 HERE
 **** THE MAIN FUNCTION CALLS THIS ONE AS ITS ENTRY POINT TO
 **** EXECUTE THE SHELL LOCALLY
 ****
 */

/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
 int exec_local_cmd_loop() {
    char *cmd_buff = malloc(SH_CMD_MAX);
    int rc = 0;
    cmd_buff_t *cmd = malloc(sizeof(cmd_buff_t));

    if (!cmd) {
        return ERR_MEMORY;
    }

    while (1) {
        printf("%s", SH_PROMPT);
        if (!fgets(cmd_buff, ARG_MAX, stdin)) {
            printf("\n");
            break;
        }

        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        rc = build_cmd_buff(cmd_buff, cmd);

        if (rc != OK) {
            if (rc == WARN_NO_CMDS) {
                printf("%s", CMD_WARN_NO_CMD);
            } else if (rc == ERR_TOO_MANY_COMMANDS) {
                printf(CMD_ERR_PIPE_LIMIT, 8);
            } else if (rc == ERR_CMD_OR_ARGS_TOO_BIG) {
                printf("Error: Command or arguments too big.\n");
            } else {
                printf("An error occurred while processing the command.\n");
            }
            continue;
        }

        if (exec_built_in_cmd(cmd) == BI_NOT_BI) {
            pid_t pid = fork();

            if (pid == 0) {
                if (execvp(cmd->argv[0], cmd->argv) == -1) {
                    exit(1);
                }
            } else {
                int status;
                waitpid(pid, &status, 0);
            }
        }

        clear_cmd_buff(cmd);
    }

    free(cmd_buff);
    free(cmd);
    return OK;
}

int last_return_code = 0;

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->_cmd_buffer = NULL;
    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff != NULL) {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL; 
    }
    return OK;  
}

int clear_cmd_buff(cmd_buff_t *cmd_buff) {
    free_cmd_buff(cmd_buff);  
    cmd_buff->argc = 0;
    for (int i = 0; i < CMD_ARGV_MAX; i++) {
        cmd_buff->argv[i] = NULL;  
    }
    return 0; 
}

Built_In_Cmds match_command(const char *input) {  
    if (!input) return BI_NOT_BI;  
    if (strcmp(input, EXIT_CMD) == 0)  
        return BI_CMD_EXIT;  
    if (strcmp(input, "dragon") == 0)  
        return BI_CMD_DRAGON;  
    if (strcmp(input, "cd") == 0)  
        return BI_CMD_CD;  
    if (strcmp(input, "rc") == 0)  
        return BI_CMD_RC;  
    return BI_NOT_BI;  
}

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd) {  
    if (!cmd || !cmd->argv[0]) return BI_NOT_BI;  

    Built_In_Cmds type = match_command(cmd->argv[0]);  

    switch (type) {  
        case BI_CMD_EXIT:  
            free_cmd_buff(cmd);  
            exit(0);  
        
        case BI_CMD_CD:  
            if (cmd->argc > 1) {  
                if (chdir(cmd->argv[1]) != 0) {  
                    perror("cd failed");  
                }  
            }  
            break;  

        case BI_CMD_DRAGON:  
            // print_dragon();  
            break;  

        case BI_CMD_RC:  
            printf("Last return code: %d\n", last_return_code);  
            break;  

        default:  
            return BI_NOT_BI;  
    }  

    return type;  
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff) {
    clear_cmd_buff(cmd_buff);  
    char *pos = cmd_line;  

    while (*pos != '\0') {
        while (isspace((unsigned char)*pos)) {  
            pos++;
        }
        if (*pos == '\0') {
            break;
        }

        char *token_start = pos;
        size_t token_length = 0;

        if (*pos == '"') {  
            token_start = ++pos;  
            while (*pos != '\0' && *pos != '"') {  
                pos++;
                token_length++;
            }
            if (*pos == '"') {  
                pos++;
            } else {  
                printf("Error: Quotation mismatch\n");
                return ERR_CMD_ARGS_BAD;
            }
        } else {  
            while (*pos != '\0' && !isspace((unsigned char)*pos)) {  
                pos++;
                token_length++;
            }
        }

        if (cmd_buff->argc >= CMD_ARGV_MAX - 1) {  
            return ERR_TOO_MANY_COMMANDS;
        }

        cmd_buff->argv[cmd_buff->argc] = strndup(token_start, token_length);  
        if (cmd_buff->argv[cmd_buff->argc] == NULL) {  
            return ERR_MEMORY;
        }
        cmd_buff->argc++;

        while (isspace((unsigned char)*pos)) {  
            pos++;
        }
    }

    cmd_buff->argv[cmd_buff->argc] = NULL;  
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    char *token;
    int cmd_count = 0;

    // Initialize command list
    clist->num = 0;

    // Tokenize the command line based on pipes
    token = strtok(cmd_line, PIPE_STRING);
    while (token != NULL && cmd_count < CMD_MAX) {
        // Remove leading and trailing whitespace
        while (isspace((unsigned char)*token)) token++;
        char *end = token + strlen(token) - 1;
        while (end > token && isspace((unsigned char)*end)) end--;
        end[1] = '\0'; // Null-terminate the string

        // Initialize the command buffer
        cmd_buff_t *cmd_buff = &clist->commands[cmd_count];

        // Tokenize the command into arguments
        cmd_buff->argc = 0;
        cmd_buff->_cmd_buffer = strdup(token); // Store the full command in _cmd_buffer

        // Split the command into arguments
        char *arg_token = strtok(cmd_buff->_cmd_buffer, " ");
        while (arg_token != NULL && cmd_buff->argc < CMD_ARGV_MAX - 1) {
            cmd_buff->argv[cmd_buff->argc++] = arg_token; // Store the argument
            arg_token = strtok(NULL, " ");
        }
        cmd_buff->argv[cmd_buff->argc] = NULL; // Null-terminate the argument list

        cmd_count++;
        token = strtok(NULL, PIPE_STRING);
    }

    clist->num = cmd_count;
    return OK;
}