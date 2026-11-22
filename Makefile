# NetBSD Makefile for xlibbar
# Created by Anlu Rodrigues De Sousa <anludesousa@gmail.com>

PROG=           xlibbar
SRCS=           xlibbar.c
OBJS=           ${SRCS:.c=.o}

CC?=            cc
CFLAGS?=        -O2
CFLAGS+=        -Wall -Wextra
CPPFLAGS+= -I/usr/X11R7/include
LDFLAGS+= -L/usr/X11R7/lib -Wl,-rpath,/usr/X11R7/lib
LDADD+=         -lX11
DPADD+=         ${LIBX11}

# Default install location. Override with:
#   make install PREFIX=/usr/pkg
PREFIX?=        /usr
BINDIR?=        ${PREFIX}/bin
INSTALL?=       install
RM?=             rm -f

.PHONY: all install clean

all: ${PROG}

${PROG}: ${OBJS}
	${CC} ${LDFLAGS} -o $@ ${OBJS} ${LDADD}

.c.o:
	${CC} ${CPPFLAGS} ${CFLAGS} -c $< -o $@

install: ${PROG}
	install -d ${DESTDIR}${BINDIR}
	${INSTALL} -m 755 ${PROG} ${DESTDIR}${BINDIR}/${PROG}

clean:
	${RM} ${PROG} ${OBJS}
