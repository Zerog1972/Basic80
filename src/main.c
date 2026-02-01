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
    printf("Commandes disponibles:\n");
    printf("  - Numérotées: ajoutent au programme (ex: 10 PRINT \"Hello\")\n");
    printf("  - LIST: affiche le programme\n");
    printf("  - RUN: exécute le programme\n");
    printf("  - CLEAR: efface le programme\n");
    printf("  - EXIT: quitte l'interpréteur\n");
    printf("  - Directes: exécutent immédiatement (PRINT, LET, etc.)\n\n");
    
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        
        /* Retirer le newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Ligne vide */
        if (strlen(line) == 0) continue;
        
        /* Commande EXIT */
        if (strcmp(line, "EXIT") == 0) {
            break;
        }
        
        /* Commande LIST */
        if (strcmp(line, "LIST") == 0) {
            l = interp->program;
            if (!l) {
                printf("Programme vide.\n");
            } else {
                while (l) {
                    printf("%d %s\n", l->lineNum, l->code);
                    l = l->next;
                }
            }
            continue;
        }
        
        /* Commande RUN */
        if (strcmp(line, "RUN") == 0) {
            runProgram(interp);
            continue;
        }
        
        /* Commande CLEAR */
        if (strcmp(line, "CLEAR") == 0) {
            freeInterpreter(interp);
            interp = createInterpreter();
            printf("Programme effacé.\n");
            continue;
        }
        
        /* Vérifier si c'est une ligne numérotée */
        lineNum = 0;
        i = 0;
        while (line[i] && isdigit(line[i])) {
            lineNum = lineNum * 10 + (line[i] - '0');
            i++;
        }
        
        if (lineNum > 0 && isspace(line[i])) {
            /* Ligne numérotée - ajouter au programme */
            while (line[i] && isspace(line[i])) i++;
            if (line[i]) {
                addLine(interp, lineNum, &line[i]);
            }
        } else {
            /* Commande immédiate */
            executeCommand(interp, line);
        }
    }
    
    printf("\nAu revoir!\n");
    freeInterpreter(interp);
    return 0;
}