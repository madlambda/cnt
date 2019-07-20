TARGET = cnt
SOURCES = main.c 
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	gcc $< -o $@

%.o: %.c 
	gcc -c $< -o $@


