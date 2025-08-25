CXX = c++

CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -IIncludes -g3 #-fsanitize=address 

CPPFILES_Y = $(addprefix src/Multiplexing/, multiplexer.cpp  cfileparser.cpp) $(addprefix src/, main.cpp)

CPPFILES_S = $(addprefix src/HttpRequest/, HttpRequest.cpp matching.cpp) \
	$(addprefix src/HttpResponse/, GET/get_handler.cpp DELETE/delete_handler.cpp) \
	$(addprefix src/Errors/, errors.cpp) \
	$(addprefix src/HttpRequest/parseRequest/, parseRequest.cpp r_body.cpp r_header.cpp r_start_line.cpp validate_path.cpp helpers.cpp )

CPPFILES_A = $(addprefix src/HttpResponse/POST/, post_handler.cpp) \
			 $(addprefix src/HttpResponse/CGI/, cgi_handler.cpp)

CPPINCLUDES = $(addprefix Includes/, multiplexer.hpp cfileparser.hpp includes.hpp)

OFILES = $(CPPFILES_Y:.cpp=.o) $(CPPFILES_S:.cpp=.o) $(CPPFILES_A:.cpp=.o)

NAME = webserv

all : $(NAME)

$(NAME) : $(OFILES) $(CPPINCLUDES)
		$(CXX) $(CXXFLAGS) $(OFILES) -o $(NAME)

clean :
	@rm -rf $(OFILES)
	@rm -rf post_body.txt

fclean : clean
	@rm -rf $(NAME)

re : fclean all
