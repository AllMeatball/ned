OBJ = \
	build/main.o \
	build/ne.o \

LDFLAGS = -g
CFLAGS = -g

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

ned: $(OBJ)
	$(CC) $^ $(LDFLAGS) -o build/$@

.PHONY: all
all: ned

.PHONY: clean
clean:
	rm -rf build/*.o
