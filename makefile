
NAME=	ircserv2

DIR=	./src/classes/
DIRO=	./obj/classes/

FILES=	Client.cpp \
		Command.cpp \
		Server.cpp \
		Parser.cpp \
		Data.cpp \
		Dispatch.cpp


SRC= $(foreach file,$(FILES),$(DIR)$(file))

OBJ = $(foreach file,$(FILES),$(DIRO)$(file:.cpp=.o))
HDIR = ./includes/
HEADERS = Client.hpp \
		Command.hpp \
		Server.hpp \
		Parser.hpp \
		Data.hpp \
		Dispatch.hpp
INCLUDES = $(foreach header,$(HEADERS),$(HDIR)$(header))

all: $(NAME)

$(NAME): $(OBJ) ./obj/main.o
	echo "Linking..."
	g++ $(OBJ) ./obj/main.o -o ircserv2

$(DIRO)%.o: $(DIR)%.cpp $(INCLUDES)
	if [ ! -d "./obj/classes" ]; then mkdir -p ./obj/classes; fi
	g++ -c $< -o $@
	
./obj/main.o: ./src/main.cpp $(INCLUDES)
	if [ ! -d "./obj" ]; then mkdir -p ./obj; fi
	g++ -c $< -o $@



fclean:
	rm -rf ./obj/classes/*.o ./obj/main.o ircserv2

.PHONY: all fclean

.PHONY: all
