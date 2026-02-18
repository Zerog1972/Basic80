/*
 * main.c - Interactive REPL entry point for Basic80
 *
 * Provides a simple read-eval-print loop that accepts numbered BASIC lines
 * (which are stored in the program), direct (un-numbered) commands (which
 * are executed immediately), and a set of meta-commands:
 *
 *   LIST  - display all program lines
 *   RUN   - execute the loaded program
 *   NEW   - erase the program and reset the interpreter
 *   SAVE "file" - save the program to a text file
 *   LOAD "file" - load a program from a text file
 *   EXIT  - quit the interpreter
 */
#include "interpreter.h"

#ifdef _WIN32
#include <windows.h>
#endif

int main(void) {
    Interpreter *interp;
    char line[1024];
    int lineNum;
    int i;
    Line *l;
    
#ifdef _WIN32
    SetConsoleOutputCP(65001); /* CP_UTF8 */
#endif

    interp = createInterpreter();
    
    printf("=== Basic80 ===\n");
    printf("Type HELP to see available commands.\n\n");
    
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        
        /* Strip the trailing newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip blank input */
        if (strlen(line) == 0) continue;
        
        /* EXIT command: quit the interpreter loop */
        if (strcmp(line, "EXIT") == 0) {
            break;
        }
        
        /* LIST command: display all loaded program lines */
        if (strcmp(line, "LIST") == 0) {
            l = interp->program;
            if (!l) {
                printf("Program is empty.\n");
            } else {
                while (l) {
                    printf("%d %s\n", l->lineNum, l->code);
                    l = l->next;
                }
            }
            continue;
        }
        
        /* RUN command: execute the loaded program */
        if (strcmp(line, "RUN") == 0) {
            runProgram(interp);
            continue;
        }
        
        /* NEW command: erase the program and reset the interpreter */
        if (strcmp(line, "NEW") == 0) {
            freeInterpreter(interp);
            interp = createInterpreter();
            printf("Program cleared.\n");
            continue;
        }
        
        /* SAVE command: save the program to a file */
        if (strncmp(line, "SAVE ", 5) == 0 || strncmp(line, "SAVE\"", 5) == 0) {
            char *filename;
            char *start;
            char *end;
            
            /* Extract the filename from between double quotes */
            start = strchr(line, '"');
            if (start) {
                start++;
                end = strchr(start, '"');
                if (end) {
                    *end = '\0';
                    filename = start;
                    if (saveProgram(interp, filename)) {
                        printf("Program saved to '%s'.\n", filename);
                    }
                } else {
                    printf("Error: Missing closing quote.\n");
                }
            } else {
                printf("Error: Filename in quotes required (e.g. SAVE \"prog.bas\").\n");
            }
            continue;
        }
        
        /* LOAD command: load a program from a file */
        if (strncmp(line, "LOAD ", 5) == 0 || strncmp(line, "LOAD\"", 5) == 0) {
            char *filename;
            char *start;
            char *end;
            
            /* Extract the filename from between double quotes */
            start = strchr(line, '"');
            if (start) {
                start++;
                end = strchr(start, '"');
                if (end) {
                    *end = '\0';
                    filename = start;
                    if (loadProgram(interp, filename)) {
                        printf("Program loaded from '%s'.\n", filename);
                    }
                } else {
                    printf("Error: Missing closing quote.\n");
                }
            } else {
                printf("Error: Filename in quotes required (e.g. LOAD \"prog.bas\").\n");
            }
            continue;
        }
        
        /* Check whether the line starts with a line number */
        lineNum = 0;
        i = 0;
        while (line[i] && isdigit(line[i])) {
            lineNum = lineNum * 10 + (line[i] - '0');
            i++;
        }
        
        if (lineNum > 0 && (isspace(line[i]) || line[i] == '\0')) {
            /* Numbered line */
            while (line[i] && isspace(line[i])) i++;
            if (line[i]) {
                /* Line has code: add or replace it in the program */
                addLine(interp, lineNum, &line[i]);
            } else {
                /* No code after the number: delete the line */
                deleteLine(interp, lineNum);
            }
        } else {
            /* Immediate (direct) command: execute right away */
            executeCommand(interp, line);
        }
    }
    
    printf("\nGoodbye!\n");
    freeInterpreter(interp);
    return 0;
}