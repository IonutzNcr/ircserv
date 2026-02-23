
NAME=	ircserv2

DIR_UTILS=	./src/utils/
DIRO_UTILS=	./obj/utils/
DIR=	./src/classes/
DIRO=	./obj/classes/

CXX= c++
CXXFLAGS=

FILES=	Client.cpp \
		Command.cpp \
		Server.cpp \
		Parser.cpp \
		Data.cpp \
		Dispatch.cpp\
		Join.cpp \
		Kick.cpp \
		RplReply.cpp \
		Channel.cpp \
		Register.cpp \
		Mode.cpp \
		Invite.cpp \
		Topic.cpp \
		Quit.cpp \
		Who.cpp

FILES_UTILS=	split.cpp


SRC= $(foreach file,$(FILES),$(DIR)$(file)) $(foreach file,$(FILES_UTILS),$(DIR_UTILS)$(file))

OBJ = $(foreach file,$(FILES),$(DIRO)$(file:.cpp=.o)) $(foreach file,$(FILES_UTILS),$(DIRO_UTILS)$(file:.cpp=.o))
HDIR = ./includes/
HEADERS = Client.hpp \
		Command.hpp \
		Server.hpp \
		Parser.hpp \
		Data.hpp \
		Dispatch.hpp \
		RplReply.hpp \
		Channel.hpp \
		split.hpp



INCLUDES = $(foreach header,$(HEADERS),$(HDIR)$(header))

all: $(NAME)

$(NAME): $(OBJ) ./obj/main.o
	echo "Linking..."
	$(CXX) $(CXXFLAGS) $(OBJ) ./obj/main.o -o ircserv2

$(DIRO)%.o: $(DIR)%.cpp $(INCLUDES)
	if [ ! -d "./obj/classes" ]; then mkdir -p ./obj/classes; fi
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
$(DIRO_UTILS)%.o: $(DIR_UTILS)%.cpp $(INCLUDES)
	if [ ! -d "./obj/utils" ]; then mkdir -p ./obj/utils; fi
	$(CXX) $(CXXFLAGS) -c $< -o $@

./obj/main.o: ./src/main.cpp $(INCLUDES)
	if [ ! -d "./obj" ]; then mkdir -p ./obj; fi
	$(CXX) $(CXXFLAGS) -c $< -o $@


fclean:
	rm -rf ./obj/classes/*.o ./obj/main.o ircserv2

re: fclean all

.PHONY: all fclean re

.PHONY: all
