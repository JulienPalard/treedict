##
## Makefile for dict
##
## Made by julien palard
## Login   <dict@mandark.fr>
##

ASPECT = GCC
PROD_CFLAGS = -O3
GCC_CFLAGS = -g3

NAME = dict
SRC = dict.c
OBJ = $(SRC:.c=.o)
CC = gcc
INCLUDE = .
DEFINE = _GNU_SOURCE
LIB = -lefence
CFLAGS = -g3 -W -Wall -ansi -pedantic -I$(INCLUDE)
RM = rm -f

$(NAME):	$(OBJ)
		$(CC) $(CFLAGS) -o $(NAME) $(OBJ) $(LIB)

install:	$(NAME)
		mkdir -p $(DESTDIR)/usr/bin/
		cp $(NAME) $(DESTDIR)/usr/bin/

all:
		@make $(NAME)

.c.o:
		$(CC) -D $(DEFINE) -c $(CFLAGS) $< -o $(<:.c=.o)

clean:
		$(RM) $(NAME) *~ \#*\# *.o *.core

re:		clean all
