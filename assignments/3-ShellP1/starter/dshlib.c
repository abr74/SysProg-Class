#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 *
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments.
 *
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 *
 *  errors returned:
 *
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the
 *                             executable name, or the arg string.
 *
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist)
{
    char *command_str;
    int command_count = 0;

    clist->num = 0;
    memset(clist->commands, 0, sizeof(clist->commands));

    command_str = strtok(cmd_line, PIPE_STRING);
    while (command_str != NULL){
        while (isspace((unsigned char)*command_str)) command_str++;

        if(*command_str == '\0'){
            command_str = strtok(NULL, PIPE_STRING);
            continue;
        }

        if (command_count >= CMD_MAX){
            return ERR_TOO_MANY_COMMANDS;
        }

        char *end = command_str + strlen(command_str) - 1;
        while (end > command_str && isspace((unsigned char)*end)) end--;
        *(end + 1) = '\0';

        if (command_count >= CMD_MAX){
            return ERR_TOO_MANY_COMMANDS;
        }

        char *exe = strtok(command_str, (const char[]){SPACE_CHAR, '\0'});
        if(exe == NULL){
            command_str = strtok(NULL, PIPE_STRING);
            continue;
        }

        strncpy(clist->commands[command_count].exe, exe, EXE_MAX - 1);
        clist->commands[command_count].exe[EXE_MAX - 1] = '\0';

        char *args = strtok(NULL, "");
        if (args !=NULL) {
            while (isspace((unsigned char)*args)) args++;
            strncpy(clist->commands[command_count].args, args, ARG_MAX - 1);
            clist->commands[command_count].args[ARG_MAX - 1] = '\0';
        } else {
            clist->commands[command_count].args[0] = '\0';
        }

        command_count++;
        command_str = strtok(NULL, PIPE_STRING);
    }

    clist->num = command_count;
    return (command_count == 0) ? WARN_NO_CMDS : OK;
}