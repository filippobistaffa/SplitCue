.PHONY: 

CMP=gcc
OPT=-Wall -Wno-unused-result -Wno-deprecated-declarations -Ofast -march=native -funroll-loops -funsafe-loop-optimizations -falign-functions=16 -falign-loops=16
FILES=split.c splitapp.c splitappwindow.c shntool/mode_split.c shntool/core_*.c shntool/format_*.c shntool/glue_*.c
LIBS=`pkg-config --cflags gtk+-3.0` -Ishntool -lpthread -lFLAC -lcurl `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0`
OUT=../splitcue

all:
	${CMP} ${OPT} ${FILES} ${LIBS} -o ${OUT}
