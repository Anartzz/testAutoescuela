OBJ=obj/server.o obj/http.o 
OBJ2=obj/htmlTemplate.o obj/queue.o obj/hashMap.o

run: server
	sudo ./server

server: $(OBJ)
	gcc -o $@ $^

obj/server.o: server.c
	gcc -c -o $@ $<

obj/http.o: http.c
	gcc -c -o $@ $<

obj/queue.o: queue.c
	gcc -c -o $@ $<

htmlTemplate: $(OBJ2)
	gcc -o $@ $^

obj/htmlTemplate.o: htmlTemplate.c
	gcc -c -o $@ $<

obj/hashMap.o: hashMap.c
	gcc -c -o $@ $<

clean:
	rm server obj/*
