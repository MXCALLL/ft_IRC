CPP = c++
CPPFLAGS = -std=c++98 #-Wall -Wextra -Werror

SRCDIR = src
INCDIR = include

SRC = main.cpp $(SRCDIR)/Server.cpp $(SRCDIR)/Client.cpp $(SRCDIR)/Utils.cpp $(SRCDIR)/cmd.cpp  $(SRCDIR)/Channel.cpp 
OBJ = $(SRC:.cpp=.o)
NAME = ircserv
HEADER = $(INCDIR)/Server.hpp $(INCDIR)/Client.hpp $(INCDIR)/Channel.hpp

all: $(NAME)

$(NAME): $(OBJ)
	$(CPP) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADER)
	$(CPP) $(CPPFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -rf $(NAME)

re: fclean all