COMPILER=GCC

# todo: object files into output path, processing c / c++ files in the same time (?), nested directories for source files (?)
rwildcard = $(wildcard $(addsuffix $2,$1)) $(foreach d,$(wildcard $(addsuffix *,$1)),$(call rwildcard,$d/,$2))
C = c
OUTPUT_PATH = out/production/Cripto/
SOURCE_PATH = src/
EXE = $(OUTPUT_PATH)crypto

ifeq ($(COMPILER), GCC)
  ifeq ($(OS),Windows_NT)
    OBJ = obj
  else
    OBJ = o
  endif
  COPT = -O2 -Wall
  CCMD = gcc
  OBJFLAG = -o
  EXEFLAG = -o
# INCLUDES = -I../.includes
  INCLUDES =
# LIBS = -lgc
  LIBS = -largtable2 -lm -lcrypto
  LIBPATH = -L/usr/local/lib
#  LIBPATH = 
  CPPFLAGS = $(COPT) -g $(INCLUDES)
  LDFLAGS = $(LIBPATH) -g $(LIBS)
  DEP = dep
else
  OBJ = obj
  COPT = /O2
  CCMD = cl
  OBJFLAG = /Fo
  EXEFLAG = /Fe
# INCLUDES = /I..\\.includes
  INCLUDES =
# LIBS = ..\\.libs\\libgc.lib
  LIBS =
  CPPFLAGS = $(COPT) /DEBUG $(INCLUDES)
  LDFLAGS = /DEBUG
endif

OBJS := $(patsubst %.$(C),%.$(OBJ),$(call rwildcard,$(SOURCE_PATH),*.$(C)))

%.$(OBJ):%.$(C)
	@echo Compiling $(basename $<)...
	$(CCMD) -c $(CPPFLAGS) $(CXXFLAGS) $< $(OBJFLAG)$@

all: $(OBJS)
	@echo Linking...
	$(CCMD) $(LDFLAGS) $^ $(EXEFLAG) $(EXE)

clean:
	rm -rf $(OBJS) $(EXE)

rebuild: clean all
#rebuild is not entirely correct
