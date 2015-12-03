all : getsections getsections_dl getsyms getsyms_dl

getsections : libobjdata.a
	@gcc -o $@ getsections.c -L./ -lobjdata -lbfd

getsections_dl : getsections_dl.c libobjdata.so
	@gcc -o $@ getsections_dl.c -L. -lobjdata -lbfd -ldl

getsyms : libobjdata.a
	@gcc -o $@ getsyms.c -L./ -lobjdata -lbfd

getsyms_dl : getsyms_dl.c libobjdata.so
	@gcc -o $@ getsyms_dl.c -L. -lobjdata -lbfd -ldl

libobjdata.so : objsect_dl.o objsym_dl.o
	@gcc -shared -o $@ $^

libobjdata.a : objsect.o objsym.o
	ar rs $@ $^
	ranlib $@

objsect_dl.o : lib/objsect.c
	@gcc -c -fPIC -o $@ $^ -lbfd -lcstring -lcstdlib

objsym_dl.o : lib/objsym.c
	@gcc -c -fPIC -o $@ $^ -lbfd -lcstring -lcstdlib

objsect.o : lib/objsect.c
	@gcc -c -o $@ $^ -lbfd -lcstring -lcstdlib

objsym.o : lib/objsym.c
	@gcc -c -o $@ $^ -lbfd -lcstring -lcstdlib
clean :
	@rm getsections_dl getsections getsyms getsyms_dl libobjdata.so libobjdata.a objsect.o objsym.o objsym_dl.o objsect_dl.o
