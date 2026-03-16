CFLAGS=-g -std=c99 -Wall -Werror -Wextra -pedantic
CPPFLAGS=-I .

hashlife: hashlife.c quadrotree.c args.c hash.c list.c

clear:
	rm hashlife
