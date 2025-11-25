# Makefile för SensorNode HTTP POST Client

# Compiler och flaggor
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
LDFLAGS = 

# Kataloger
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = build

# Målprogrammets namn
TARGET = $(BINDIR)/sensornode2.0

# Källfiler
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Headers
HEADERS = $(wildcard $(INCDIR)/*.h)

# Standard målet
all: directories $(TARGET)

# Skapa nödvändiga kataloger
directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

# Länka objektfiler till exekverbar fil
$(TARGET): $(OBJECTS)
	@echo "Länker $(TARGET)..."
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Kompilering klar! Kör med: ./$(TARGET)"

# Kompilera C-filer till objektfiler
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	@echo "Kompilerar $<..."
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Rensa byggfiler
clean:
	@echo "Rensar byggfiler..."
	rm -rf $(OBJDIR) $(BINDIR) response.log

# Kör programmet med standardparametrar
run: $(TARGET)
	./$(TARGET)

# Kör programmet med log handler
run-log: $(TARGET)
	./$(TARGET) log 25.3

# Kör programmet med hjälp
help: $(TARGET)
	./$(TARGET) --help

# Kör programmet med random temperatur
run-random: $(TARGET)
	./$(TARGET) --random

# Kör med valgrind för minnesanalys
valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TARGET) --interval 10

# Kortare valgrind-körning (10 sekunder)
valgrind-short: $(TARGET)
	timeout 10 valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET) --interval 5

# Visa information om Make-målen
info:
	@echo "Tillgängliga mål:"
	@echo "  all       - Kompilera hela programmet (standard)"
	@echo "  clean     - Rensa alla byggfiler"
	@echo "  run       - Kompilera och kör med standardparametrar"
	@echo "  run-log   - Kör med log handler och temperatur 25.3"
	@echo "  run-random - Kör med slumpmässig temperatur"
	@echo "  valgrind  - Kör med valgrind minnesanalys"
	@echo "  valgrind-short - Kort valgrind-test (10 sek)"
	@echo "  help      - Visa programmets hjälp"
	@echo "  info      - Visa denna information"
	@echo ""
	@echo "Källfiler:"
	@echo "  $(SOURCES)"
	@echo ""
	@echo "Objektfiler:"
	@echo "  $(OBJECTS)"

# Debug-version med extra debug-information
debug: CFLAGS += -DDEBUG -O0
debug: $(TARGET)

# Release-version med optimering
release: CFLAGS += -O2 -DNDEBUG
release: clean $(TARGET)

# Installera programmet (kräver sudo för /usr/local/bin)
install: $(TARGET)
	@echo "Installerar $(TARGET) till /usr/local/bin/..."
	sudo cp $(TARGET) /usr/local/bin/sensornode2.0
	@echo "Installation klar! Kör med: sensornode2.0"

# Avinstallera programmet
uninstall:
	@echo "Avinstallerar sensornode2.0 från /usr/local/bin/..."
	sudo rm -f /usr/local/bin/sensornode2.0
	@echo "Avinstallation klar!"

# Phony targets (dessa är inte filer)
.PHONY: all clean run run-log run-random valgrind valgrind-short help info debug release install uninstall directories

# Visa vilka filer som kommer kompileras
show-files:
	@echo "Källfiler som kommer kompileras:"
	@for file in $(SOURCES); do echo "  $$file"; done
	@echo ""
	@echo "Header-filer:"
	@for file in $(HEADERS); do echo "  $$file"; done