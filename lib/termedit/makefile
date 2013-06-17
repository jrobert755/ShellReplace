OBJECTS = stringhelp.o termedit.o
LIB=../libtermedit.a
 
all: $(OBJECTS) $(LIB)

clean:
	rm *.o
	rm $(LIB)

$(LIB): $(OBJECTS)
	ar -cvr $(LIB) $(OBJECTS)

$(OBJECTS): %.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
