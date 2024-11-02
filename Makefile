NAME = webserv
CXX = c++
CXXFLAGS := -Wall -Wextra -Werror -std=c++17

INCLUDE_DIR = include
LDFLAGS := 
SOURCE_DIR = source
SOURCES := main.cpp HttpServer.cpp ServerConfig.cpp Client.cpp Socket.cpp Request.cpp Response.cpp Utils.cpp Location.cpp
OBJECTS := $(addprefix $(SOURCE_DIR)/,$(SOURCES:.cpp=.o))

target asan: CXXFLAGS += -fsanitize=address,undefined -g
target debug: CXXFLAGS += -g

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)

re: fclean all
asan: re
debug: re

.PHONY: all re clean fclean asan debug
