#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SZ 50

// Prototypes
void usage(char *);
void print_buff(char *, int);
int setup_buff(char *, char *, int);
int count_words(char *, int, int);
void print_word(char *start, char *end);

// Function to set up the buffer by processing the user input string
int setup_buff(char *buff, char *user_str, int len) {
    int i = 0, j = 0;

    // Loop through the input string and copy it into the buffer
    while (*(user_str + i) != '\0' && j < len - 1) {
        if (*(user_str + i) == ' ' || *(user_str + i) == '\t') {
            // Avoid multiple consecutive spaces by checking the previous character
            if (j == 0 || *(buff + j - 1) == ' ') {
                i++;
                continue;
            }
            *(buff + j) = ' ';
        } else {
            *(buff + j) = *(user_str + i);
        }
        i++;
        j++;
    }

    // Fill the remaining buffer space with dots to mark unused space
    while (j < len) {
        *(buff + j) = '.';
        j++;
    }

    return i;
}

// Function to print the contents of the buffer
void print_buff(char *buff, int len) {
    printf("Buffer:  ");
    for (int i = 0; i < len; i++) {
        putchar(*(buff + i));
    }
    putchar('\n');
}

// Function to display usage instructions for the program
void usage(char *exename) {
    printf("usage: %s [-h|c|r|w|x] \"string\" [other args]\n", exename);
}

// Function to count words in the buffer
int count_words(char *buff, int len, int str_len) {
    int word_count = 0;
    for (int i = 0; i < str_len; i++) {
        if (*(buff + i) == ' ') {
            word_count++;
        }
    }
    return word_count + 1;  // Return word count, assuming words are separated by spaces
}

// Function to print a word from the buffer, given the start and end pointers
void print_word(char *start, char *end) {
    int word_len = end - start;
    printf("%d. ", word_len);
    for (char *p = start; p < end; p++) {
        putchar(*p);
    }
    printf(" (%d)\n", word_len);
}

int main(int argc, char *argv[]) {

    char *buff;             // Placeholder for the internal buffer
    char *input_string;     // Holds the string provided by the user on the command line
    char opt;               // Used to capture the user option from the command line
    int rc;                 // Used for return codes
    int user_str_len;       // Length of the user-supplied string

    // TODO: #1. WHY IS THIS SAFE, aka what if argv[1] does not exist?
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    // If argc < 2 or argv[1] does not start with '-', the program is not properly invoked.
    // In this case, we print the usage information and exit with an error.
    if ((argc < 2) || (*argv[1] != '-')) {
        usage(argv[0]);
        exit(1);
    }

    opt = (char)*(argv[1] + 1);  // Get the option flag (e.g., 'c', 'r', 'w')

    // Handle the help flag and exit normally
    if (opt == 'h') {
        usage(argv[0]);
        exit(0);
    }

    // TODO: #2 Document the purpose of the if statement below
    //      PLACE A COMMENT BLOCK HERE EXPLAINING
    // The check for argc < 3 ensures that the user has provided the string argument after the flag.
    // If not, the program prints usage instructions and exits.
    if (argc < 3) {
        usage(argv[0]);
        exit(1);
    }

    input_string = argv[2];  // Capture the user input string

    // TODO: #3 Allocate space for the buffer using malloc and
    //          handle error if malloc fails by exiting with a 
    //          return code of 99
    // CODE GOES HERE FOR #3
    // The buffer is dynamically allocated here using malloc to store the user's input string.
    // If malloc fails (i.e., returns NULL), we print an error message and exit with a return code of 99.
    buff = malloc(BUFFER_SZ);
    if (buff == NULL) {
        printf("Error: Memory allocation failed\n");
        exit(99);
    }

    // Set up the buffer with the user string, and check if the setup was successful
    user_str_len = setup_buff(buff, input_string, BUFFER_SZ);
    if (user_str_len < 0) {
        printf("Error setting up buffer, error = %d", user_str_len);
        exit(2);
    }

    // Switch statement to handle different options provided by the user
    switch (opt) {
        case 'c':  // Count the words in the input string
            rc = count_words(buff, BUFFER_SZ, user_str_len);
            if (rc < 0) {
                printf("Error counting words, rc = %d", rc);
                exit(2);
            }
            printf("Word Count: %d\n", rc);
            break;

        // TODO: #5 Implement the other cases for 'r' and 'w' by extending
        //       the case statement options
        case 'r':  // Reverse the string in the buffer
            {
                char *start = buff;
                char *end = buff + user_str_len - 1;
                while (start < end) {
                    // Swap characters at the start and end
                    char temp = *start;
                    *start = *end;
                    *end = temp;
                    start++;
                    end--;
                }
                printf("Reversed String: ");
                print_buff(buff, user_str_len);
            }
            break;

        case 'w':  // Print the words from the buffer
            {
                printf("Word Print\n");
                printf("----------\n");
                int word_count = 1;
                char *word_start = buff;
                for (int i = 0; i < user_str_len; i++) {
                    if (*(buff + i) == ' ' || *(buff + i) == '.') {
                        // Print each word and its length
                        if (word_start != buff + i) {
                            printf("%d. ", word_count);
                            print_word(word_start, buff + i);
                            word_count++;
                        }
                        word_start = buff + i + 1;
                    }
                }
            }
            break;

        default:  // Invalid option, display usage instructions
            usage(argv[0]);
            exit(1);
    }

    // TODO: #6 Don't forget to free your buffer before exiting
    // We free the allocated buffer memory before exiting to avoid memory leaks.
    free(buff);
    print_buff(buff, BUFFER_SZ);
    exit(0);
}

// TODO: #7 Notice all of the helper functions provided in the 
//       starter take both the buffer as well as the length.  Why
//       do you think providing both the pointer and the length
//       is a good practice, after all we know from main() that 
//       the buff variable will have exactly 50 bytes?
//
//       PLACE YOUR ANSWER HERE
//
// Providing both the pointer and the length ensures that the function can safely handle portions of the buffer without exceeding the allocated size. 
// While the buffer is allocated to 50 bytes, the actual data stored within it may be smaller (e.g., based on the user input length), so it's important to pass both to 
// prevent buffer overflows or accessing out-of-bounds memory. This also makes the function more reusable and flexible, capable of handling buffers of varying sizes.
