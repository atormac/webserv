NAME = webserv
CXX = c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17

INCLUDE_DIR = include
LDFLAGS := 
SOURCE_DIR = source
SOURCES := main.cpp Defines.cpp HttpServer.cpp HttpServerParser.cpp ServerConfig.cpp Client.cpp Socket.cpp \
	   Request.cpp Response.cpp Utils.cpp Location.cpp \
	   Str.cpp Io.cpp Cgi.cpp Cookies.cpp Logger.cpp

OBJ_DIR := obj
OBJECTS := $(addprefix $(OBJ_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

$(shell mkdir -p $(OBJ_DIR))

#NPROCS := $(shell grep -c ^processor /proc/cpuinfo)
NPROCS := 6

target asan_flags: CXXFLAGS += -fsanitize=address,undefined -g
target debug_flags: CXXFLAGS += -gdwarf-4 -fstandalone-debug

all: $(NAME)

bonus: all

$(NAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

#%.o: %.cpp
$(OBJ_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)

re:
	@make fclean
	@make all -j$(NPROCS)

asan_flags: all
debug_flags: all

debug:
	@make fclean
	@make debug_flags -j$(NPROCS)
asan:
	@make fclean
	@make asan_flags -j$(NPROCS)

run: asan
	./webserv

.PHONY: all bonus re clean fclean asan asan_flags debug debug_flags run
