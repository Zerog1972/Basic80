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
    printf("  - NEW: efface le programme\n");
    printf("  - SAVE \"fichier.bas\": sauvegarde le programme\n");
    printf("  - LOAD \"fichier.bas\": charge un programme\n");
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
        
        /* Commande NEW */
        if (strcmp(line, "NEW") == 0) {
            freeInterpreter(interp);
            interp = createInterpreter();
            printf("Programme effacé.\n");
            continue;
        }
        
        /* Commande SAVE */
        if (strncmp(line, "SAVE ", 5) == 0 || strncmp(line, "SAVE\"", 5) == 0) {
            char *filename;
            char *start;
            char *end;
            
            /* Trouver le nom du fichier entre guillemets */
            start = strchr(line, '"');
            if (start) {
                start++;
                end = strchr(start, '"');
                if (end) {
                    *end = '\0';
                    filename = start;
                    if (saveProgram(interp, filename)) {
                        printf("Programme sauvegardé dans '%s'.\n", filename);
                    }
                } else {
                    printf("Erreur: Guillemet fermant manquant.\n");
                }
            } else {
                printf("Erreur: Nom de fichier entre guillemets requis (ex: SAVE \"prog.bas\").\n");
            }
            continue;
        }
        
        /* Commande LOAD */
        if (strncmp(line, "LOAD ", 5) == 0 || strncmp(line, "LOAD\"", 5) == 0) {
            char *filename;
            char *start;
            char *end;
            
            /* Trouver le nom du fichier entre guillemets */
            start = strchr(line, '"');
            if (start) {
                start++;
                end = strchr(start, '"');
                if (end) {
                    *end = '\0';
                    filename = start;
                    if (loadProgram(interp, filename)) {
                        printf("Programme chargé depuis '%s'.\n", filename);
                    }
                } else {
                    printf("Erreur: Guillemet fermant manquant.\n");
                }
            } else {
                printf("Erreur: Nom de fichier entre guillemets requis (ex: LOAD \"prog.bas\").\n");
            }
            continue;
        }
        
        /* Vérifier si c'est une ligne numérotée */
        lineNum = 0;
        i = 0;
        while (line[i] && isdigit(line[i])) {
            lineNum = lineNum * 10 + (line[i] - '0');
            i++;
        }
        
        if (lineNum > 0 && (isspace(line[i]) || line[i] == '\0')) {
            /* Ligne numérotée */
            while (line[i] && isspace(line[i])) i++;
            if (line[i]) {
                /* Il y a du code - ajouter/modifier la ligne */
                addLine(interp, lineNum, &line[i]);
            } else {
                /* Pas de code - supprimer la ligne */
                deleteLine(interp, lineNum);
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