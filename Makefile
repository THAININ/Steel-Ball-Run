CC = g++
CFLAGS = -Wall -std=c++11
LIBS = -lGLEW -lGL -lGLU -lglfw -lSOIL -lglut

SRC = test.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = orbita

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LIBS)

.cpp.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
