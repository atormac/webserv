NAME = webserv
CXX = c++
CXXFLAGS := -Wall -Wextra -Werror

SOURCE_DIR = source
SOURCES := main.cpp HttpServer.cpp Client.cpp Socket.cpp ServersManager.cpp
OBJECTS := $(addprefix $(SOURCE_DIR)/,$(SOURCES:.cpp=.o))

target asan: CXXFLAGS += -fsanitize=address,undefined -g
target debug: CXXFLAGS += -g

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -Iinclude -o $@ $^

%.o: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(NAME)

re: fclean all
asan: re
debug: re

.PHONY: all re clean fclean asan debug
