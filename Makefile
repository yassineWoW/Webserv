CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -IIncludes

CPPFILES_Y = $(addprefix src/, multiplexer.cpp main.cpp cfileparser.cpp)

CPPFILES_S = $(addprefix src/HttpRequest/, HttpRequest.cpp) \
	$(addprefix src/HttpRequest/parseRequest/, parseRequest.cpp r_body.cpp r_header.cpp r_start_line.cpp helpers.cpp)

CPPFILES_A = $(addprefix src/, )

CPPINCLUDES = $(addprefix Includes/, multiplexer.hpp cfileparser.hpp includes.hpp)

OFILES = $(CPPFILES_Y:.cpp=.o) $(CPPFILES_S:.cpp=.o) $(CPPFILES_A:.cpp=.o)

NAME = webserv

all : $(NAME)

$(NAME) : $(OFILES) $(CPPINCLUDES)
		$(CXX) $(CXXFLAGS) $(OFILES) -o $(NAME)

clean :
	@rm -rf $(OFILES)

fclean : clean
	@rm -rf $(NAME)

re : fclean all
