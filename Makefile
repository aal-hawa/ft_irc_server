NAME = ircserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS_DIR = sources
INCS_DIR = includes

SRCS = $(SRCS_DIR)/main.cpp \
       $(SRCS_DIR)/Server.cpp \
       $(SRCS_DIR)/Client.cpp \
       $(SRCS_DIR)/Channel.cpp \
       $(SRCS_DIR)/Message.cpp \
       $(SRCS_DIR)/Commands.cpp \
       $(SRCS_DIR)/Utils.cpp

OBJS = $(SRCS:.cpp=.o)

INCS = -I $(INCS_DIR)

all: $(NAME)

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) $(OBJS) -o $(NAME)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
