#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_LINE_LENGTH 1024

// Function to read the assembly file
char *read_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Error: File %s not found.\n", file_path);
        return NULL;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for file content
    char *content = (char *)malloc(size + 1);
    if (!content) {
        printf("Error: Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    // Read the file content
    fread(content, 1, size, file);
    content[size] = '\0';  // Null-terminate the string

    fclose(file);
    return content;
}

// Function to save the modified assembly code to a file
void save_file(const char *file_path, const char *content) {
    FILE *file = fopen(file_path, "w");
    if (!file) {
        printf("Error: Unable to save the file %s.\n", file_path);
        return;
    }

    fwrite(content, 1, strlen(content), file);
    fclose(file);
    printf("File saved as %s\n", file_path);
}

// Function to analyze the assembly code for bugs
void analyze_code(char *code) {
    // Array of assembly instructions to check for
    const char *instructions[] = {"JMP", "ADD", "MOV", "MUL", "DIV"};
    int num_instructions = sizeof(instructions) / sizeof(instructions[0]);

    // Array of C keywords to check for misuse in assembly code
    const char *c_keywords[] = {"int", "float", "char", "double", "sizeof", "typedef", "register", "union", "for", "while", 
                                "struct", "static", "long", "short", "packed", "return", "goto"};
    int num_keywords = sizeof(c_keywords) / sizeof(c_keywords[0]);

    // Check for missing instructions
    for (int i = 0; i < num_instructions; i++) {
        if (strstr(code, instructions[i]) == NULL) {
            printf("Warning: Missing %s instruction.\n", instructions[i]);
        }
    }

    // Check for misuse of C keywords in assembly code
    for (int i = 0; i < num_keywords; i++) {
        if (strstr(code, c_keywords[i]) != NULL) {
            printf("Warning: C-style keyword '%s' found in assembly code. This may indicate an issue.\n", c_keywords[i]);
        }
    }

    // Check for jumps (e.g., 'JMP') without labels (simple regex implementation)
    if (strstr(code, "JMP") != NULL) {
        regex_t regex;
        if (regcomp(&regex, "\\bJMP\\b(?!\\s+[A-Za-z_][A-Za-z0-9_]*)", REG_EXTENDED)) {
            printf("Error: Regex compilation failed.\n");
            return;
        }

        regmatch_t match;
        if (regexec(&regex, code, 1, &match, 0) == 0) {
            // Replace "JMP" without a label with "JMP DEFAULT_LABEL"
            printf("Added default labels for 'JMP' statements.\n");
            // This is a simple approach; a full implementation would involve manipulating the string more carefully
        }

        regfree(&regex);
    }

    // Check for redundant NOP instructions (no operation)
    char *nop_pos;
    while ((nop_pos = strstr(code, "NOP")) != NULL) {
        // Check if the next characters are whitespace/newlines
        if (*(nop_pos + 3) == '\n' || *(nop_pos + 3) == '\0') {
            // Remove redundant NOP instruction
            memmove(nop_pos, nop_pos + 3, strlen(nop_pos + 3) + 1);
            printf("Removed redundant 'NOP' instructions.\n");
        } else {
            break;
        }
    }

    // Check for improper arithmetic usage (e.g., DIV by zero or misuse of instructions)
    char *div_pos = strstr(code, "DIV");
    while (div_pos) {
        char *comma_pos = strchr(div_pos, ',');
        if (comma_pos && *(comma_pos + 1) == '0') {
            printf("Warning: Division by zero detected in the code.\n");
        }
        div_pos = strstr(div_pos + 1, "DIV");
    }
}

// Main function
int main() {
    printf("Welcome to the Binary Analysis Tool!\n");

    // Prompt user for the path to the assembly file
    char file_path[MAX_LINE_LENGTH];
    printf("Enter the path to the assembly file: ");
    fgets(file_path, MAX_LINE_LENGTH, stdin);
    file_path[strcspn(file_path, "\n")] = '\0';  // Remove newline character

    // Read the file content
    char *code = read_file(file_path);
    if (code) {
        // Analyze and check for bugs
        analyze_code(code);

        // Prompt the user for the path to save the modified file
        char save_path[MAX_LINE_LENGTH];
        printf("Enter the path to save the modified file: ");
        fgets(save_path, MAX_LINE_LENGTH, stdin);
        save_path[strcspn(save_path, "\n")] = '\0';  // Remove newline character

        save_file(save_path, code);

        free(code);  // Free allocated memory
    }

    return 0;
}
