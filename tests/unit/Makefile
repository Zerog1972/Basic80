# Makefile pour les tests unitaires Basic80

CC = clang
CFLAGS = -std=c89 -pedantic -Wall -g -I../../include
SRCDIR = ../../src
TESTDIR = .
OBJDIR = obj

# Fichiers sources du projet
SOURCES = $(SRCDIR)/lexer.c $(SRCDIR)/variables.c $(SRCDIR)/expression.c \
          $(SRCDIR)/interpreter.c $(SRCDIR)/commands.c $(SRCDIR)/control_flow.c

# Fichiers de tests
TEST_SOURCES = test_framework.c test_lexer.c test_variables.c \
               test_expression.c test_interpreter.c run_unit_tests.c

# Fichiers objets
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TEST_OBJECTS = $(TEST_SOURCES:%.c=$(OBJDIR)/%.o)

# Exécutable
TARGET = run_unit_tests.exe

# Règle par défaut
all: $(OBJDIR) $(TARGET)

# Créer le répertoire obj
$(OBJDIR):
	@if not exist $(OBJDIR) mkdir $(OBJDIR)

# Compilation des sources du projet
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compilation des tests
$(OBJDIR)/%.o: $(TESTDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Lien de l'exécutable
$(TARGET): $(OBJECTS) $(TEST_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ -lm

# Exécuter les tests
test: $(TARGET)
	.\$(TARGET)

# Nettoyage
clean:
	@if exist $(OBJDIR) rd /s /q $(OBJDIR)
	@if exist $(TARGET) del $(TARGET)

.PHONY: all test clean
