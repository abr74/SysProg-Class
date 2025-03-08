#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "dragon.h"
#include "dshlib.h"

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



int close_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff) {  
        free_cmd_buff(cmd_buff);  
        free(cmd_buff);  
    }
    return OK;
}

int build_cmd_list(char *cmd_line, command_list_t *clist) {
    memset(clist, 0, sizeof(command_list_t));

    char *current_ptr = cmd_line;
    while (*current_ptr != '\0' && isspace((unsigned char)*current_ptr)) {
        current_ptr++;
    }

    if (*current_ptr == '\0') {
        return WARN_NO_CMDS;
    }

    char *saved;
    char *segment = strtok_r(current_ptr, PIPE_STRING, &saved);

    for (int count = 0; segment != NULL; count++) {
        if (count >= CMD_ARGV_MAX) {
            return ERR_TOO_MANY_COMMANDS;
        }

        cmd_buff_t *temp_cmd = (cmd_buff_t *)malloc(sizeof(cmd_buff_t));
        if (!temp_cmd) {
            return ERR_MEMORY;
        }

        alloc_cmd_buff(temp_cmd);
        int status = build_cmd_buff(segment, temp_cmd);
        
        if (status != OK) {
            free(temp_cmd);
            return status;
        }

        clist->commands[count] = *temp_cmd;
        free(temp_cmd);

        clist->num++;
        segment = strtok_r(NULL, PIPE_STRING, &saved);
    }

    return OK;
}

int free_cmd_list(command_list_t *clist) {
    if (!clist) {
        return ERR_MEMORY;
    }

    int total_cmds = clist->num;
    for (int index = 0; index < total_cmds; index++) {
        free_cmd_buff(&clist->commands[index]);
    }

    clist->num = 0;
    return OK;
}

int exec_cmd(cmd_buff_t *cmd) {
    Built_In_Cmds cmd_type = exec_built_in_cmd(cmd);
    
    if (cmd_type == BI_NOT_BI) {
        pid_t child_pid = fork();
        
        if (child_pid < 0) {
            perror("Error: Forking process failed");
            return -1;
        } else if (child_pid == 0) {
            // Handle input redirection
            if (cmd -> input_filepath) {
                int input_fd = open(cmd->input_filepath, O_RDONLY);
                if (input_fd == -1) {
                    perror("Error: Unable to open input file");
                    exit(EXIT_FAILURE);
                }
                if (dup2(input_fd, STDIN_FILENO) == -1) {
                    perror("Error: Redirecting input failed");
                    exit(EXIT_FAILURE);
                }
                close(input_fd);
            }
            
            // Handle output redirection
            if (cmd->output_filepath) {
                int output_flags = O_WRONLY | O_CREAT;
                output_flags |= (cmd->is_append) ? O_APPEND : O_TRUNC;
                
                int output_fd = open(cmd->output_filepath, output_flags, 0644);
                if (output_fd == -1) {
                    perror("Error: Unable to open output file");
                    exit(EXIT_FAILURE);
                }
                if (dup2(output_fd, STDOUT_FILENO) == -1) {
                    perror("Error: Redirecting output failed");
                    exit(EXIT_FAILURE);
                }
                close(output_fd);
            }
            
            // Execute command
            if (execvp(cmd->argv[0], cmd->argv) == -1) {
                perror("Error: Command execution failed");
                exit(EXIT_FAILURE);
            }
        } else {
            int exit_status;
            if (waitpid(child_pid, &exit_status, 0) == -1) {
                perror("Error: Waiting for child process failed");
                return -1;
            }
            if (WIFEXITED(exit_status)) {
                return WEXITSTATUS(exit_status);
            }
            return -1;
        }
    }
    return OK;
}

int execute_pipeline(command_list_t *clist) {
    if (clist->num == 1) {
        return exec_cmd(&clist->commands[0]);
    }

    int pipe_fds[clist->num - 1][2];
    pid_t child_pids[clist->num];

    for (int i = 0; i < clist->num - 1; i++) {
        if (pipe(pipe_fds[i]) != 0) {
            perror("Error creating pipe");
            return -1;
        }
    }

    for (int i = 0; i < clist->num; i++) {
        if ((child_pids[i] = fork()) < 0) {
            perror("Fork failed");
            return -1;
        }

        if (child_pids[i] == 0) { 
            if (i == 0) {
                if (clist->commands[i].input_filepath) {
                    int input_fd = open(clist->commands[i].input_filepath, O_RDONLY);
                    if (input_fd < 0) {
                        perror("Failed to open input file");
                        exit(EXIT_FAILURE);
                    }
                    dup2(input_fd, STDIN_FILENO);
                    close(input_fd);
                }
            } else {
                dup2(pipe_fds[i - 1][0], STDIN_FILENO);
            }

            if (i == clist->num - 1) {
                if (clist->commands[i].output_filepath) {
                    int out_flags = O_WRONLY | O_CREAT | (clist->commands[i].is_append ? O_APPEND : O_TRUNC);
                    int output_fd = open(clist->commands[i].output_filepath, out_flags, 0644);
                    if (output_fd < 0) {
                        perror("Failed to open output file");
                        exit(EXIT_FAILURE);
                    }
                    dup2(output_fd, STDOUT_FILENO);
                    close(output_fd);
                }
            } else {
                dup2(pipe_fds[i][1], STDOUT_FILENO);
            }

            for (int j = 0; j < clist->num - 1; j++) {
                close(pipe_fds[j][0]);
                close(pipe_fds[j][1]);
            }

            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("Execution error");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < clist->num - 1; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }

    for (int i = 0; i < clist->num; i++) {
        int exit_status;
        waitpid(child_pids[i], &exit_status, 0);
    }

    return OK;
}
 
// --------------------------------------------------------------------------

int alloc_cmd_buff(cmd_buff_t *cmd_buff) {
    cmd_buff->argc = 0;
    cmd_buff->_cmd_buffer = NULL;
    memset(cmd_buff->argv, 0, CMD_ARGV_MAX * sizeof(char *));
    cmd_buff->input_filepath = NULL;
    cmd_buff->output_filepath = NULL;
    cmd_buff->is_append = false;

    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff) {
    if (cmd_buff) {
        if (cmd_buff->_cmd_buffer != NULL) {
            free(cmd_buff->_cmd_buffer);
            cmd_buff->_cmd_buffer = NULL;
        }

        if (cmd_buff->input_filepath != NULL) {
            free(cmd_buff->input_filepath);
            cmd_buff->input_filepath = NULL;
        }

        if (cmd_buff->output_filepath != NULL) {
            free(cmd_buff->output_filepath);
            cmd_buff->output_filepath = NULL;
        }
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
            print_dragon();  
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

        if (*pos == '<') {
            pos++;  
            while (isspace((unsigned char)*pos)) {  
                pos++;
            }

            char *file_start = pos;
            size_t file_len = 0;

            while (*pos != '\0' && !isspace((unsigned char)*pos) && *pos != '<' && *pos != '>') {
                pos++;
                file_len++;
            }

            cmd_buff->input_filepath = strndup(file_start, file_len);
            if (cmd_buff->input_filepath == NULL) {
                return ERR_MEMORY;
            }
            continue;
        }

        if (*pos == '>') {
            pos++;
            if (*pos == '>') {  
                cmd_buff->is_append = true;
                pos++;
            } else {
                cmd_buff->is_append = false;
            }

            while (isspace((unsigned char)*pos)) {  
                pos++;
            }

            char *file_start = pos;
            size_t file_len = 0;

            while (*pos != '\0' && !isspace((unsigned char)*pos) && *pos != '<' && *pos != '>') {
                pos++;
                file_len++;
            }

            cmd_buff->output_filepath = strndup(file_start, file_len);
            if (cmd_buff->output_filepath == NULL) {
                return ERR_MEMORY;
            }
            continue;
        }

        char *token_start = pos;
        size_t token_length = 0;
        char quote_char = '\0';

        if (*pos == '"' || *pos == '\'') {  
            quote_char = *pos;
            token_start = ++pos;  

            while (*pos != '\0' && *pos != quote_char) {  
                pos++;
                token_length++;
            }

            if (*pos == '\0') {  
                printf("Error: Unmatched quote\n");
                return ERR_CMD_ARGS_BAD;
            }
            pos++;
        } else {  
            while (*pos != '\0' && !isspace((unsigned char)*pos) && *pos != '<' && *pos != '>') {  
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


//  --------------------------------------------------------------------------

int exec_local_cmd_loop() {
    int status;
    char cmd_buffer[SH_CMD_MAX + 1];
    command_list_t cmd_list;

    while (true) {
        printf("%s", SH_PROMPT);
        
        if (!fgets(cmd_buffer, SH_CMD_MAX, stdin)) {
            puts(""); 
            break;
        }

        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0'; 

        if (strcmp(cmd_buffer, EXIT_CMD) == 0) {
            break;
        }

        status = build_cmd_list(cmd_buffer, &cmd_list);
        
        if (status != OK) {
            switch (status) {
                case WARN_NO_CMDS:
                    puts(CMD_WARN_NO_CMD);
                    continue;
                case ERR_TOO_MANY_COMMANDS:
                    puts(CMD_ERR_PIPE_LIMIT);
                    continue;
                case ERR_MEMORY:
                    fputs("Memory allocation error.\n", stderr);
                    continue;
            }
        }

        execute_pipeline(&cmd_list);
        free_cmd_list(&cmd_list);
    }

    return status;
}