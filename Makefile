all: libsbmemlib.a  app create_memory_sb destroy_memory_sb

libsbmemlib.a:  sbmemlib.c
	gcc -Wall -c -ggdb3 sbmemlib.c -lrt -lpthread
	ar -cvq libsbmemlib.a sbmemlib.o
	ranlib libsbmemlib.a

app: app.c
	gcc -Wall -o app -ggdb3 app.c -L. -lsbmemlib -lpthread -lrt

create_memory_sb: create_memory_sb.c
	gcc -Wall -o create_memory_sb -ggdb3 create_memory_sb.c -L. -lsbmemlib -lpthread -lrt

destroy_memory_sb: destroy_memory_sb.c
	gcc -Wall -o destroy_memory_sb -ggdb3 destroy_memory_sb.c -L. -lsbmemlib -lpthread -lrt

clean: 
	rm -fr *.o *.a *~ a.out  app sbmemlib.o sbmemlib.a libsbmemlib.a  create_memory_sb destroy_memory_sb
