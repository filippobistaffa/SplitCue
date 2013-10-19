.PHONY: 
	
all:
	gcc split.c shntool/mode_split.c shntool/core_*.c shntool/format_*.c shntool/glue_*.c -O3 `pkg-config --cflags gtk+-3.0` -Wall -Ishntool -lpthread -lFLAC -lcurl `pkg-config --cflags gtk+-3.0` `pkg-config --libs gtk+-3.0` -o ../splitcue
