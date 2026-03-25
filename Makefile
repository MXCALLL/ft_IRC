CPP = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98

SRCDIR = src
INCDIR = include

SRC = main.cpp $(SRCDIR)/Server.cpp $(SRCDIR)/Client.cpp $(SRCDIR)/Utils.cpp
OBJ = $(SRC:.cpp=.o)
NAME = IRCSERVER
HEADER = $(INCDIR)/Server.hpp $(INCDIR)/Client.hpp

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(OBJ) $(CPPFLAGS) -o $(NAME)

%.o: %.cpp $(HEADER)
	$(CPP) $(CPPFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all

.PHONY: all clean fclean re