NAME = webserv
CXX = c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17

INCLUDE_DIR = include
LDFLAGS := 
SOURCE_DIR = source
SOURCES := main.cpp HttpServer.cpp ServerConfig.cpp Client.cpp Socket.cpp Request.cpp Response.cpp Utils.cpp Location.cpp

OBJ_DIR := obj
#OBJECTS := $(addprefix $(SOURCE_DIR)/,$(SOURCES:.cpp=.o))
OBJECTS := $(addprefix $(OBJ_DIR)/, $(notdir $(SOURCES:.cpp=.o)))

$(shell mkdir -p $(OBJ_DIR))

target asan: CXXFLAGS += -fsanitize=address,undefined -g
target debug: CXXFLAGS += -g

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

re: fclean all
asan: re
debug: re

.PHONY: all re clean fclean asan debug
