.SUFFIXES: .cpp .hpp

# Programs
SHELL 	= bash
CC     	= g++
LD	= ld
RM 	= rm
ECHO	= /bin/echo
CAT	= cat
PRINTF	= printf
SED	= sed
DOXYGEN = doxygen
GPROF = gprof
PYTHON  = python
DOT = dot
######################################
# Project Name (generate executable with this name)
TARGET = cs251_base

# Project Paths
PROJECT_ROOT=./
EXTERNAL_ROOT=$(PROJECT_ROOT)/external
SRCDIR = $(PROJECT_ROOT)/src
OBJDIR = $(PROJECT_ROOT)/obj
BINDIR = $(PROJECT_ROOT)/bin
DOCDIR = $(PROJECT_ROOT)/doc
PROFILEDIR = $(PROJECT_ROOT)/Profile
REPORTDIR = $(PROJECT_ROOT)/Report
# Library Paths
BOX2D_ROOT=$(EXTERNAL_ROOT)
GLUI_ROOT=/usr
GL_ROOT=/usr/include/

#Libraries
LIBS = -lBox2D -lglui -lglut -lGLU -lGL

# Compiler and Linker flags
CPPFLAGS =-pg -O3 -Wall -fno-strict-aliasing
CPPFLAGS+=-I $(BOX2D_ROOT)/include -I $(GLUI_ROOT)/include
LDFLAGS+=-L $(BOX2D_ROOT)/lib -L $(GLUI_ROOT)/lib

######################################

NO_COLOR=\e[0m
OK_COLOR=\e[1;32m
ERR_COLOR=\e[1;31m
WARN_COLOR=\e[1;33m
MESG_COLOR=\e[1;34m
FILE_COLOR=\e[1;37m

OK_STRING="[OK]"
ERR_STRING="[ERRORS]"
WARN_STRING="[WARNINGS]"
OK_FMT="${OK_COLOR}%30s\n${NO_COLOR}"
ERR_FMT="${ERR_COLOR}%30s\n${NO_COLOR}"
WARN_FMT="${WARN_COLOR}%30s\n${NO_COLOR}"
######################################

SRCS := $(wildcard $(SRCDIR)/*.cpp)
INCS := $(wildcard $(SRCDIR)/*.hpp)
OBJS := $(SRCS:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)


.PHONY: all setup codeDoc clean profile release report distclean

all: setup $(BINDIR)/$(TARGET)

release: setup $(BINDIR)/$(TARGET) codeDoc report
	

setup:
	@$(ECHO) "Setting up compilation..."
	@mkdir -p obj
	@mkdir -p bin

$(BINDIR)/$(TARGET): $(OBJS)
	@$(PRINTF) "$(MESG_COLOR)Building executable:$(NO_COLOR) $(FILE_COLOR) %16s$(NO_COLOR)" "$(notdir $@)"
	@$(CC) -pg -o $@ $(LDFLAGS) $(OBJS) $(LIBS) 2> temp.log || touch temp.err
	@if test -e temp.err; \
	then $(PRINTF) $(ERR_FMT) $(ERR_STRING) && $(CAT) temp.log; \
	elif test -s temp.log; \
	then $(PRINTF) $(WARN_FMT) $(WARN_STRING) && $(CAT) temp.log; \
	else $(PRINTF) $(OK_FMT) $(OK_STRING); \
	fi;
	@$(RM) -f temp.log temp.err

-include -include $(OBJS:.o=.d)

$(OBJS): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@$(PRINTF) "$(MESG_COLOR)Compiling: $(NO_COLOR) $(FILE_COLOR) %25s$(NO_COLOR)" "$(notdir $<)"
	@$(CC) $(CPPFLAGS) -c $< -o $@ -MD 2> temp.log || touch temp.err
	@if test -e temp.err; \
	then $(PRINTF) $(ERR_FMT) $(ERR_STRING) && $(CAT) temp.log; \
	elif test -s temp.log; \
	then $(PRINTF) $(WARN_FMT) $(WARN_STRING) && $(CAT) temp.log; \
	else printf "${OK_COLOR}%30s\n${NO_COLOR}" "[OK]"; \
	fi;
	@$(RM) -f temp.log temp.err

codeDoc:
	@$(ECHO) -n "Generating Doxygen Documentation for 8 Ball Pool...  "
	@$(RM) -rf $(DOCDIR)/html &> /dev/null
	@$(DOXYGEN) $(DOCDIR)/Doxyfile 2 &> /dev/null
	@$(ECHO) "Done!"

#Please run the executable file from the main directory once to generate gmon.out
profile:
	@$(ECHO) -n "Generating Profiling Data...  "
	@$(GPROF) $(BINDIR)/cs251_base  > $(PROFILEDIR)/call_graph.txt 
	@$(PYTHON) $(PROFILEDIR)/gprof2dot.py $(PROFILEDIR)/call_graph.txt > $(PROFILEDIR)/call_graph.dot 
	@$(DOT) -Tpng $(PROFILEDIR)/call_graph.dot -o $(PROFILEDIR)/call_graph.png 
	@$(ECHO) "Done!"

report:
	@$(ECHO) -n "Generating Project Report and Presentation in Report directory...  "
	@pdflatex --output-directory=$(REPORTDIR) $(REPORTDIR)/report.tex &> /dev/null && \
	cd $(REPORTDIR) && \
	bibtex report.aux &> /dev/null
	@pdflatex --output-directory=$(REPORTDIR) $(REPORTDIR)/report.tex &> /dev/null
	@pdflatex --output-directory=$(REPORTDIR) $(REPORTDIR)/report.tex &> /dev/null
	@$(RM) $(REPORTDIR)/report.aux
	@$(RM) $(REPORTDIR)/report.log
	@$(RM) $(REPORTDIR)/report.out
	@$(RM) $(REPORTDIR)/report.blg
	@$(RM) $(REPORTDIR)/report.bbl
	@pdflatex --output-directory=$(REPORTDIR) $(REPORTDIR)/presentation.tex &> /dev/null
	@$(RM) $(REPORTDIR)/presentation.aux
	@$(RM) $(REPORTDIR)/presentation.log
	@$(RM) $(REPORTDIR)/presentation.out
	@$(RM) $(REPORTDIR)/presentation.nav
	@$(RM) $(REPORTDIR)/presentation.snm
	@$(RM) $(REPORTDIR)/presentation.toc
	@$(ECHO) "Done"  

clean:
	@$(ECHO) -n "Cleaning up..."
	@$(RM) -rf $(OBJDIR) *~ $(DEPS) $(SRCDIR)/*~
	@$(ECHO) "Done"

distclean: clean
	@$(RM) -rf $(BINDIR) $(DOCDIR)/html $(PROFILEDIR)/call_graph.*  

