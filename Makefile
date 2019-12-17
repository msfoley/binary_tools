SRCDIR := src
INCLUDEDIR := include
BUILDDIR := build
OBJDIR := $(BUILDDIR)/obj
BINDIR := $(BUILDDIR)/bin

CC := gcc
CFLAGS := -g -O0 -Wall -I$(INCLUDEDIR)
LDFLAGS :=
LDLIBS :=

SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))

BINARIES := bincat binreplace bindiff binpatch
BINOBJ := $(patsubst %,$(OBJDIR)/%.o,$(BINARIES))
BIN := $(addprefix $(BINDIR)/,$(BINARIES))

COMMONOBJ := $(filter-out $(BINOBJ),$(OBJS))

.PHONY: all clean

all: $(BIN)

$(BUILDDIR):
	mkdir -v $@

$(OBJDIR) $(BINDIR): $(BUILDDIR)
	mkdir -v $@

$(BINDIR)/%: $(OBJDIR)/%.o $(COMMONOBJ) | $(BINDIR)
	$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) -r $(BUILDDIR)

#print-% : ; @echo $* = $($*)
