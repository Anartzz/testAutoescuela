OBJ=obj/server.o obj/http.o 
OBJ2=obj/htmlTemplate.o obj/queue.o obj/hashMap.o obj/stack.o

run: server
	sudo ./server

server: $(OBJ)
	gcc -g -o $@ $^

obj/server.o: server.c
	gcc -g -c -o $@ $<

obj/http.o: http.c
	gcc -g -c -o $@ $<

obj/queue.o: queue.c
	gcc -g -c -o $@ $<

htmlTemplate: $(OBJ2)
	gcc -g -o $@ $^

obj/htmlTemplate.o: htmlTemplate.c
	gcc -g -c -o $@ $<

obj/hashMap.o: hashMap.c
	gcc -g -c -o $@ $<

obj/stack.o: stack.c
	gcc -g -c -o $@ $<

clean:
	rm server obj/*
