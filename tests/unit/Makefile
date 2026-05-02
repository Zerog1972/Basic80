# Makefile pour les tests unitaires Basic80

CC = clang
CFLAGS = -std=c89 -pedantic -Wall -g -I../../include
SRCDIR = ../../src
TESTDIR = .
OBJDIR = obj

# Fichiers sources du projet
SOURCES = $(SRCDIR)/lexer.c $(SRCDIR)/vars.c $(SRCDIR)/expr.c \
          $(SRCDIR)/interp.c $(SRCDIR)/commands.c $(SRCDIR)/ctrlflow.c

# Fichiers de tests
TEST_SOURCES = tstfrmwk.c tstlexer.c tst_vars.c \
               tst_expr.c tstintrp.c runtests.c

# Fichiers objets
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TEST_OBJECTS = $(TEST_SOURCES:%.c=$(OBJDIR)/%.o)

# Exécutable
TARGET = runtests.exe

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
