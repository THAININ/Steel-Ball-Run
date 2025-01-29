CC = g++
CFLAGS = -Wall -std=c++11
LIBS = -lGL -lGLU -lglut -lGLEW -lSOIL -lm

SRC = main.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = a

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
