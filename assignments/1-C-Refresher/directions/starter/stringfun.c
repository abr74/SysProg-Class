#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define BUFFER_SZ 50

//prototypes
void usage(char *);
void print_buff(char *, int);
int  setup_buff(char *, char *, int);

//prototypes for functions to handle required functionality
int  count_words(char *, int, int);
//add additional prototypes here
void reverse_string(char *, int);
void print_words(char *, int, int);
void replace_string(char *, char *, char *, int);


int setup_buff(char *buff, char *user_str, int len){
    //TODO: #4:  Implement the setup buff as per the directions
    // Check if the user string is too long
    int i = 0, j = 0;
    char last_char = ' ';

    // Copy non-whitespace characters to the buffer
    while (*(user_str + i) != '\0') {
        char current_char = *(user_str + i);
        if (current_char != ' ' && current_char != '\t') {
            *(buff + j) = current_char;
            j++;
            last_char = current_char;
        } else if (last_char != ' ') {
            *(buff + j) = ' ';
            j++;
            last_char = ' ';
        }
        i++;
    }

    // Fill the rest of the buffer with '.'
    while (j < len) {
        *(buff + j) = '.';
        j++;
    }

    // Return the length of the user string
    if (i > len) {
        return -1; // User string is too large
    }
    return j; // Return the length of the processed string
}

void print_buff(char *buff, int len){
    printf("Buffer:  ");
    for (int i=0; i<len; i++){
        putchar(*(buff+i));
    }
    putchar('\n');
}

void usage(char *exename){
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);

}

int count_words(char *buff, int len, int str_len){
    //YOU MUST IMPLEMENT
    int count = 0;
    int in_word = 0;

    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) != ' ' && *(buff + i) != '.') {
            if (!in_word) {
                in_word = 1; // We are in a word
                count++;
            }
        } else {
            in_word = 0; // We are not in a word
        }
    }
    return count;
}

//ADD OTHER HELPER FUNCTIONS HERE FOR OTHER REQUIRED PROGRAM OPTIONS
void reverse_string(char *buff, int str_len) {
    int start = 0;
    int end = str_len - 1;

    while (start < end) {
        char temp = *(buff + start);
        *(buff + start) = *(buff + end);
        *(buff + end) = temp;
        start++;
        end--;
    }
}

void print_words(char *buff, int len, int str_len) {
    printf("Word Print\n");
    printf("----------\n");
    int word_index = 1;
    int in_word = 0;
    int word_start = 0;

    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) != ' ' && *(buff + i) != '.') {
            if (!in_word) {
                in_word = 1; // Start of a new word
                word_start = i;
            }
        } else {
            if (in_word) {
                in_word = 0; // End of a word
                int word_length = i - word_start;
                printf("%d. ", word_index++);
                for (int j = word_start; j < i; j++) {
                    putchar(*(buff + j));
                }
                printf(" (%d)\n", word_length);
            }
        }
    }
    // Handle the last word if the string does not end with a space
    if (in_word) {
        int word_length = str_len - word_start;
        printf("%d. ", word_index++);
        for (int j = word_start; j < str_len; j++) {
            putchar(*(buff + j));
        }
        printf(" (%d)\n", word_length);
    }
}

void replace_string(char *buff, char *old_str, char *new_str, int len) {
    char *pos = strstr(buff, old_str); // Find the first occurrence of old_str
    if (pos != NULL) {
        int old_len = strlen(old_str);
        int new_len = strlen(new_str);
        int remaining_len = strlen(buff) - (pos - buff) - old_len;

        // Check if the new string can fit in the buffer
        if (new_len > old_len) {
            // If the new string is longer, we need to shift the remaining part
            if (remaining_len + new_len - old_len >= len) {
                printf("Error: Replacement string is too long for the buffer.\n");
                return;
            }
            // Shift the remaining part of the string to the right
            memmove(pos + new_len, pos + old_len, remaining_len + 1); // +1 for null terminator
        } else if (new_len < old_len) {
            // If the new string is shorter, we need to shift the remaining part to the left
            memmove(pos + new_len, pos + old_len, remaining_len + 1); // +1 for null terminator
        }

        // Copy the new string into the position of the old string
        memcpy(pos, new_str, new_len);
    }
}


int main(int argc, char *argv[]){

    char *buff;             //placehoder for the internal buffer
    char *input_string;     //holds the string provided by the user on cmd line
    char opt;               //used to capture user option from cmd line
    int  rc;                //used for return codes
    int  user_str_len;      //length of user supplied string

    //TODO:  #1. WHY IS THIS SAFE, aka what if arv[1] does not exist?
    // If argc < 2 or argv[1] does not start with '-', the program is not properly invoked.
    // In this case, we print the usage information and exit with an error.
    if ((argc < 2) || (*argv[1] != '-')){
        usage(argv[0]);
        exit(1);
    }

    opt = *(argv[1]+1);   //get the option flag

    //handle the help flag and then exit normally
    if (opt == 'h'){
        usage(argv[0]);
        exit(0);
    }

    //WE NOW WILL HANDLE THE REQUIRED OPERATIONS

    //TODO:  #2 Document the purpose of the if statement below
    // The check for argc < 3 ensures that the user has provided the string argument after the flag.
    // If not, the program prints usage instructions and exits.
    if (argc < 3){
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2]; //capture the user input string

    //TODO:  #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3

    buff = (char *)malloc(BUFFER_SZ);
    if (buff == NULL) {
        exit(99); // Handle malloc failure
    }


    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);     //see todos
    if (user_str_len < 0){
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    switch (opt){
        case 'c':
            rc = count_words(buff, BUFFER_SZ, user_str_len);  //you need to implement
            if (rc < 0){
                printf("Error counting words, rc = %d", rc);
                free(buff);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        //TODO:  #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':
            reverse_string(buff, user_str_len);
            printf("Reversed String: ");
            for (int i = 0; i < user_str_len; i++) {
                putchar(*(buff + i));
            }
            putchar('\n');
            break;

        case 'w':
            print_words(buff, BUFFER_SZ, user_str_len);
            break;

        case 'x':
            if (argc < 5) {
                printf("Error: Not enough arguments for -x\n");
                free(buff);
                exit(1);
            }
            replace_string(buff, argv[3], argv[4], BUFFER_SZ);
            printf("Modified String: ");
            print_buff(buff, user_str_len); // Print the modified buffer
            break;

        default:
            usage(argv[0]);
            exit(1);
    }

    //TODO:  #6 Dont forget to free your buffer before exiting
    print_buff(buff,BUFFER_SZ);
    free(buff);
    exit(0);
}

//TODO:  #7  Notice all of the helper functions provided in the 
//          starter take both the buffer as well as the length.  Why
//          do you think providing both the pointer and the length
//          is a good practice, after all we know from main() that 
//          the buff variable will have exactly 50 bytes?
//  
// Providing both the pointer and the length ensures that the function can safely handle portions of the buffer without exceeding the allocated size. 
// While the buffer is allocated to 50 bytes, the actual data stored within it may be smaller (e.g., based on the user input length), so it's important to pass both to 
// prevent buffer overflows or accessing out-of-bounds memory. This also makes the function more reusable and flexible, capable of handling buffers of varying sizes.