Acquacchi : bitboards.o board.o data.o evaluation.o hash.o init.o makemove.o \
	misc.o movegen.o perft.o search.o acquacchi.o protocols.o validate.o
		cc -o Acquacchi bitboards.o board.o data.o evaluation.o hash.o init.o makemove.o \
	misc.o movegen.o perft.o search.o acquacchi.o protocols.o validate.o

bitboards.o : bitboards.c defs.h data.h protos.h
		cc -c bitboards.c
board.o : board.c defs.h data.h protos.h
		cc -c board.c
data.o : data.c defs.h data.h protos.h
		cc -c data.c
evaluation.o : evaluation.c defs.h data.h protos.h
		cc -c evaluation.c
hash.o : hash.c defs.h data.h protos.h
		cc -c hash.c
init.o : init.c defs.h data.h protos.h
		cc -c init.c
makemove.o : makemove.c defs.h data.h protos.h
		cc -c makemove.c
misc.o : misc.c defs.h data.h protos.h
		cc -c misc.c
movegen.o : movegen.c defs.h data.h protos.h
		cc -c movegen.c
perft.o : perft.c defs.h data.h protos.h
		cc -c perft.c
search.o : search.c defs.h data.h protos.h
		cc -c search.c
acquacchi.o : acquacchi.c defs.h data.h protos.h
		cc -c acquacchi.c
protocols.o : protocols.c defs.h data.h protos.h
		cc -c protocols.c
validate.o : validate.c defs.h data.h protos.h
		cc -c validate.c 
clean :
		rm Acquacci bitboards.o board.o data.o evaluation.o hash.o init.o makemove.o \
    misc.o movegen.o perft.o search.o acquacchi.o protocols.o validate.o

# OBJECT_FILES = \
# 	attack.o \
# 	bitboards.o \
# 	board.o \
# 	data.o \
# 	evaluate.o \
# 	hashkeys.o \
# 	init.o \
# 	io.o \
# 	makemove.o \
# 	misc.o \
# 	movegen.o \
# 	perft.o \
# 	pvtable.o \
# 	search.o \
# 	acquacchi.o \
# 	uci.o \
# 	validate.o \
# 	xboard.o

# all: Acquacchi

# tscp: $(OBJECT_FILES)
# 	g++ -O3 -o Acquacchi $(OBJECT_FILES)

# %.o: %.c data.h defs.h protos.h
# 	g++ -O3 -x c -c $< -o $@

# clean:
# 	rm -f *.o
# 	rm -f Acquacchi

# all:
# 	gcc xboard.c acquacchi.c uci.c evaluate.c pvtable.c init.c bitboards.c hashkeys.c board.c data.c attack.c io.c movegen.c validate.c makemove.c perft.c search.c misc.c -o Acquacchi -O2