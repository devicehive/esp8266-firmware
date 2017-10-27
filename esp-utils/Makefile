CXX				?= g++
CFLAGS			+= -c -Wall
LDFLAGS			+= -Os
LIBS			=
OBJDIR			= build
SOURCES			= $(wildcard common/*.cpp)
COMMONOBJECTS	= $(addprefix $(OBJDIR)/, $(SOURCES:%.cpp=%.o))
MAINS			= $(wildcard *.cpp)
ALLOBJECTS		= $(COMMONOBJECTS) $(addprefix $(OBJDIR)/, $(MAINS:%.cpp=%.o))
ESPTERMINAL		= $(OBJDIR)/esp-terminal
ESPFLASHER		= $(OBJDIR)/esp-flasher
ifeq ($(OS),Windows_NT)
	LDFLAGS		+= -static -s
else
	ifneq ($(shell uname -s),Darwin)
		LDFLAGS		+= -s
		LIBS		= -pthread
	endif
endif

all: $(SOURCES) $(ESPTERMINAL) $(ESPFLASHER)
    
$(ESPTERMINAL): $(ALLOBJECTS)
	@echo "LD $@"
	@$(CXX) $(LDFLAGS) $(LIBS) $(COMMONOBJECTS) $@.o -o $@

$(ESPFLASHER): $(ALLOBJECTS)
	@echo "LD $@"
	@$(CXX) $(LDFLAGS) $(LIBS) $(COMMONOBJECTS) $@.o -o $@

$(OBJDIR)/%.o: %.cpp
	@echo "CXX $<"
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS) $< -o $@

rebuild: clean all    

clean:
	@rm -rf $(OBJDIR)
