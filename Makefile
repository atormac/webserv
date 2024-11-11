MAKEFLAGS += -j8
NAME = webserv
CXX = c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17

INCLUDE_DIR = include
LDFLAGS := 
SOURCE_DIR = source
SOURCES := main.cpp HttpServer.cpp ServerConfig.cpp Client.cpp Socket.cpp Request.cpp Response.cpp Utils.cpp Location.cpp Str.cpp

OBJ_DIR := obj
#OBJECTS := $(addprefix $(SOURCE_DIR)/,$(SOURCES:.cpp=.o))
OBJECTS := $(addprefix $(OBJ_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

$(shell mkdir -p $(OBJ_DIR))

target asan_flags: CXXFLAGS += -fsanitize=address,undefined -g
target debug_flags: CXXFLAGS += -g

all: $(NAME)

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
	$(MAKE) fclean
	$(MAKE) all

asan_flags: all
debug_flags: all

debug:
	$(MAKE) fclean
	$(MAKE) debug_flags
asan:
	$(MAKE) fclean
	$(MAKE) asan_flags
run:
	$(MAKE) fclean
	$(MAKE) asan_flags
	./webserv

.PHONY: all re clean fclean asan asan_flags debug debug_flags run
