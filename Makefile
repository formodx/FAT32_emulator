CC = gcc
CFLAGS = -std=c11 -pedantic
INCDIR = include
SRCDIR = src
OBJDIR = obj
TARGET = fat32


sources := $(wildcard $(SRCDIR)/*.c)
objects = $(sources:$(SRCDIR)/%.c=$(OBJDIR)/%.o)


.PHONY: all clean


all: $(TARGET)


$(TARGET): $(objects)
	$(CC) $(CFLAGS) -o $@ $^


$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c $(CFLAGS) -I $(INCDIR) -o $@ $<


$(OBJDIR):
	mkdir -p $(OBJDIR)


clean:
	rm -rf $(OBJDIR) $(TARGET)


rebuild: clean all