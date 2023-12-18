OBJ=server.o http.o


run: server
	./$<

server: $(OBJ)
	gcc -o $@ $^ 

server.o: server.c
	gcc -c -o $@ $< 

http.o: http.c
	gcc -c -o $@ $< 

clean:
	rm -rf *.o server
