CFLAGS=-g -std=c99 -Wall -Werror -Wextra -pedantic
CPPFLAGS=-I include  
VPATH=src

hashlife: hashlife.c quadrotree.c args.c hash.c list.c test.c

clear:
	rm hashlife
