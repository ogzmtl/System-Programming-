CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic

# Directories
SRCDIR = src
SERV_SOURCE = server
INCDIR = includes
BUILDDIR = build
BUILD_SERVER = serverbuild
TARGET = operateArrays
SERVER = BibakBOXServer

CLIENT_SOURCE = client
BUILD_CLIENT= clientbuild
CLIENT = BibakBOXClient

# Source and object files
# SRCS = $(wildcard $(SRCDIR)/*.c)
# OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))
SERVER_SOURCE = $(wildcard $(SERV_SOURCE)/*.c)
OBJS_SERVER = $(patsubst $(SERV_SOURCE)/%.c,$(BUILD_SERVER)/%.o,$(SERVER_SOURCE))
CLIENT_SRC = $(wildcard $(CLIENT_SOURCE)/*.c)
OBJS_CLIENT = $(patsubst $(CLIENT_SOURCE)/%.c,$(BUILD_CLIENT)/%.o,$(CLIENT_SRC))
# DEPS = $(wildcard $(INCDIR)/*.h)
all : $(SERVER) $(CLIENT)

$(SERVER): $(OBJS_SERVER)
	gcc $(CXXFLAGS) -o $@ $^

$(CLIENT): $(OBJS_CLIENT)
	gcc $(CXXFLAGS) -o $@ $^

# $(TARGET): $(OBJS)
# 	g++ $(CXXFLAGS) -o $@ $^

$(BUILD_SERVER)/%.o: $(SERV_SOURCE)/%.c
	gcc $(CXXFLAGS) -c -o $@ $< 
$(BUILD_CLIENT)/%.o: $(CLIENT_SOURCE)/%.c
	gcc $(CXXFLAGS) -c -o $@ $< 

# $(BUILDDIR)/%.o: $(SRCDIR)/%.c $(DEPS)
# 	g++ $(CXXFLAGS) -c -o $@ $< -I$(INCDIR)

# Clean the build directory
clean:
	rm $(BUILD_SERVER)/*.o 
	rm $(BUILD_CLIENT)/*.o 
	rm $(SERVER)
	rm $(CLIENT)
