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
 *   EDIT n - edit a stored line by replacing part of its text
 *   SAVE "file" - save the program to a text file
 *   LOAD "file" - load a program from a text file
 *   EXIT  - quit the interpreter
 */
#include "interp.h"
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

/*
 * Check that a single path component (file or directory name) conforms to
 * the DOS 8.3 format:
 *   - Name part  : 1 to 8 alphanumeric characters (or $%'-_@~`!(){}^#&)
 *   - Extension  : 0 to 3 characters of the same set, separated by one dot
 *   - No second dot is allowed
 * Returns 1 if valid, 0 otherwise.
 */
static int isValidName83(const char *name) {
    const char *allowed = "$%'-_@~`!(){}^#&";
    const char *dot;
    size_t namelen;
    size_t extlen;
    size_t i;

    if (!name || *name == '\0') return 0;

    dot = strchr(name, '.');
    if (dot) {
        namelen = (size_t)(dot - name);
        extlen  = strlen(dot + 1);
        /* A second dot in the extension is forbidden */
        if (strchr(dot + 1, '.') != NULL) return 0;
    } else {
        namelen = strlen(name);
        extlen  = 0;
    }

    if (namelen == 0 || namelen > 8) return 0;
    if (extlen > 3)                  return 0;

    for (i = 0; i < namelen; i++) {
        char c = name[i];
        if (!isalnum((unsigned char)c) && !strchr(allowed, c)) return 0;
    }
    for (i = 0; i < extlen; i++) {
        char c = dot[1 + i];
        if (!isalnum((unsigned char)c) && !strchr(allowed, c)) return 0;
    }

    return 1;
}

/*
 * Check that every component of a path (directories and final filename)
 * conforms to the DOS 8.3 format.  Both '/' and '\' are accepted as
 * separators.  Absolute path prefixes (drive letters like "C:" and a
 * leading separator) are skipped automatically.
 * Returns 1 if all components are valid, 0 otherwise.
 * On failure, *badComponent is set to point to the offending component
 * inside the local buffer – copy it before the buffer goes out of scope.
 */
static int isValidPath83(const char *path, char *badComponent, size_t badSize) {
    char buf[256];
    char *p;
    char *sep;

    strncpy(buf, path, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    p = buf;

    /* Skip an optional drive letter (e.g. "C:") */
    if (isalpha((unsigned char)p[0]) && p[1] == ':') {
        p += 2;
    }

    /* Skip a leading separator */
    if (*p == '/' || *p == '\\') p++;

    while (*p) {
        /* Find the next separator */
        sep = p;
        while (*sep && *sep != '/' && *sep != '\\') sep++;

        if (sep > p) {
            char saved = *sep;
            *sep = '\0';
            if (!isValidName83(p)) {
                if (badComponent && badSize > 0) {
                    strncpy(badComponent, p, badSize - 1);
                    badComponent[badSize - 1] = '\0';
                }
                return 0;
            }
            *sep = saved;
        }

        if (*sep == '\0') break;
        p = sep + 1;
    }

    return 1;
}

/* Handle a SAVE or LOAD meta-command: extract the quoted filename and
 * call saveProgram / loadProgram accordingly. */
static void handleFileCommand(Interpreter *interp, const char *line, int isSave) {
    const char *start;
    const char *end;
    char filename[256];
    char badComp[64];
    size_t len;

    start = strchr(line, '"');
    if (!start) {
        printf("Error: Filename in quotes required (e.g. %s \"prog.bas\").\n",
               isSave ? "SAVE" : "LOAD");
        return;
    }
    start++;
    end = strchr(start, '"');
    if (!end) {
        printf("Error: Missing closing quote.\n");
        return;
    }
    len = (size_t)(end - start);
    if (len >= sizeof(filename)) len = sizeof(filename) - 1;
    memcpy(filename, start, len);
    filename[len] = '\0';

    /* Validate every component of the path against the DOS 8.3 format */
    if (!isValidPath83(filename, badComp, sizeof(badComp))) {
        printf("Error: '%s' does not conform to the DOS 8.3 format.\n", badComp);
        printf("  Rules: name <= 8 chars, extension <= 3 chars, no spaces.\n");
        return;
    }

    if (isSave) {
        if (saveProgram(interp, filename))
            printf("Program saved to '%s'.\n", filename);
    } else {
        if (loadProgram(interp, filename))
            printf("Program loaded from '%s'.\n", filename);
    }
}

/* Find a stored line by number in the sorted program list. */
static Line* findStoredLine(Interpreter *interp, int lineNum) {
    Line *line;

    line = interp->program;
    while (line && line->lineNum < lineNum) {
        line = line->next;
    }
    if (line && line->lineNum == lineNum) {
        return line;
    }
    return NULL;
}

/* Queue prefilled text into the Windows console input buffer so the user
 * can edit an existing line directly at the prompt. */
#ifdef _WIN32
static int queueConsolePrefill(const char *text) {
    HANDLE hIn;
    DWORD mode;
    size_t len;
    INPUT_RECORD *records;
    DWORD written;
    size_t i;

    hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) return 0;
    if (!GetConsoleMode(hIn, &mode)) return 0;

    len = strlen(text);
    if (len == 0) return 1;

    records = (INPUT_RECORD*)malloc(sizeof(INPUT_RECORD) * (len * 2));
    if (!records) return 0;

    for (i = 0; i < len; i++) {
        memset(&records[i * 2], 0, sizeof(INPUT_RECORD));
        records[i * 2].EventType = KEY_EVENT;
        records[i * 2].Event.KeyEvent.bKeyDown = TRUE;
        records[i * 2].Event.KeyEvent.wRepeatCount = 1;
        records[i * 2].Event.KeyEvent.uChar.AsciiChar = text[i];

        memset(&records[i * 2 + 1], 0, sizeof(INPUT_RECORD));
        records[i * 2 + 1].EventType = KEY_EVENT;
        records[i * 2 + 1].Event.KeyEvent.bKeyDown = FALSE;
        records[i * 2 + 1].Event.KeyEvent.wRepeatCount = 1;
        records[i * 2 + 1].Event.KeyEvent.uChar.AsciiChar = text[i];
    }

    written = 0;
    if (!WriteConsoleInput(hIn, records, (DWORD)(len * 2), &written)) {
        free(records);
        return 0;
    }

    free(records);
    return written == (DWORD)(len * 2);
}
#endif

/* EDIT <lineNumber>: open a directly editable prefilled line so the user
 * can modify it without retyping it entirely. */
static void handleEditCommand(Interpreter *interp, const char *line) {
    const char *p;
    char *endPtr;
    long parsed;
    int lineNum;
    Line *target;
    char editedCode[1024];

    p = line + 4;
    while (*p && isspace((unsigned char)*p)) p++;

    if (*p == '\0') {
        printf("Usage: EDIT <lineNumber>\n");
        return;
    }

    parsed = strtol(p, &endPtr, 10);
    if (endPtr == p || parsed <= 0 || parsed > 32767) {
        printf("Error: Invalid line number.\n");
        return;
    }
    while (*endPtr && isspace((unsigned char)*endPtr)) endPtr++;
    if (*endPtr != '\0') {
        printf("Usage: EDIT <lineNumber>\n");
        return;
    }

    lineNum = (int)parsed;
    target = findStoredLine(interp, lineNum);
    if (!target) {
        printf("Error: Line %d not found.\n", lineNum);
        return;
    }

    printf("Edit line %d (Enter to keep, empty after erase to delete):\n", lineNum);
    printf("%d ", lineNum);
    fflush(stdout);

#ifdef _WIN32
    (void)queueConsolePrefill(target->code);
#endif

    if (!fgets(editedCode, sizeof(editedCode), stdin)) {
        clearerr(stdin);
        printf("EDIT canceled.\n");
        return;
    }
    editedCode[strcspn(editedCode, "\n")] = '\0';

    if (strcmp(editedCode, target->code) == 0) {
        printf("Line unchanged.\n");
        return;
    }

    if (editedCode[0] == '\0') {
        deleteLine(interp, lineNum);
        printf("Line %d deleted.\n", lineNum);
        return;
    }

    addLine(interp, lineNum, editedCode);
    printf("Line updated: %d %s\n", lineNum, editedCode);
}

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

    printf("  ____               _         ___   ___  \n");
    printf(" | __ )  __ _  ___  (_)  ___  ( _ ) / _ \\ \n");
    printf(" |  _ \\ / _` |/ __| | | / __| / _ \\| | | |\n");
    printf(" | |_) | (_| |\\__ \\ | || (__ | (_) | |_| |\n");
    printf(" |____/ \\__,_||___/ |_| \\___| \\___/ \\___/ \n");
    printf("                      by ZeroG1972 - 2026\n\n");
    printf("Welcome to Basic80 interpreter for a vintage-style BASIC dialect.\n");

    printf("\n\nType HELP to see available commands.\n\n");
    
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        
        /* Strip the trailing newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip blank input */
        if (line[0] == '\0') continue;
        
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
            clock_t startTime;
            clock_t endTime;
            double elapsedSeconds;
            int elapsedMinutes;
            double remainingSeconds;

            startTime = clock();
            runProgram(interp);
            endTime = clock();

            if (startTime != (clock_t)-1 && endTime != (clock_t)-1) {
                elapsedSeconds = (double)(endTime - startTime) / CLOCKS_PER_SEC;
                elapsedMinutes = (int)(elapsedSeconds / 60.0);
                remainingSeconds = elapsedSeconds - (elapsedMinutes * 60.0);
                printf("Program executed in %d min %.3f s.\n", elapsedMinutes, remainingSeconds);
            }
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
        if (strncmp(line, "SAVE", 4) == 0 && (line[4] == ' ' || line[4] == '"')) {
            handleFileCommand(interp, line, 1);
            continue;
        }
        
        /* LOAD command: load a program from a file */
        if (strncmp(line, "LOAD", 4) == 0 && (line[4] == ' ' || line[4] == '"')) {
            handleFileCommand(interp, line, 0);
            continue;
        }

        /* EDIT command: interactively edit an existing line */
        if (strncmp(line, "EDIT", 4) == 0 && (line[4] == '\0' || isspace((unsigned char)line[4]))) {
            handleEditCommand(interp, line);
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