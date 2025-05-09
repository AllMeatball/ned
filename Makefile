OBJ = \
	src/main.o \
	src/ne.o \

LDFLAGS = -g
CFLAGS = -g

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

ned: $(OBJ)
	mkdir -p build/
	$(CC) $^ $(LDFLAGS) -o build/$@

.PHONY: all
all: ned

.PHONY: clean
clean:
	rm -rf src/*.o build/ned
