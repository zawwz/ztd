IDIR=include
SRCDIR=src
ODIR=obj
ODIR_SHARED=obj_so

NAME = libztd

CC=g++
CXXFLAGS= -I$(IDIR) -Wall -std=c++17 -O2

$(shell mkdir -p $(ODIR))
$(shell mkdir -p $(ODIR_SHARED))

# automatically finds .h and .hpp
DEPS = $(shell if [ -n "$(ls $(IDIR))" ] ; then ls $(IDIR)/*.hpp $(IDIR)/*.h 2>/dev/null ; fi)

# automatically finds .c and .cpp and makes the corresponding .o rule
OBJ = $(shell ls $(SRCDIR)/*.cpp $(SRCDIR)/*.c 2>/dev/null | sed 's|\.cpp|.o|g;s|\.c|.o|g;s|$(SRCDIR)/|$(ODIR)/|g')

# automatically finds .c and .cpp and makes the corresponding .o rule
OBJ_SHARED = $(shell ls $(SRCDIR)/*.cpp $(SRCDIR)/*.c 2>/dev/null | sed 's|\.cpp|.o|g;s|\.c|.o|g;s|$(SRCDIR)/|$(ODIR_SHARED)/|g')


$(ODIR)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) $(CXXFLAGS) -c -o $@ $<

$(ODIR_SHARED)/%.o: $(SRCDIR)/%.c $(DEPS)
	$(CC) $(CXXFLAGS) -fPIC -o $@ $<

$(ODIR_SHARED)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	$(CC) $(CXXFLAGS) -fPIC -c -o $@ $<

all: static shared

static: $(OBJ)
	ar rcs libztd.a $^

shared: $(OBJ_SHARED)
	$(CC) -shared -o libztd.so $^

install:
	mkdir -p $(INSTALL)/usr/lib
	cp libztd.a libztd.so $(INSTALL)/usr/lib
	mkdir -p $(INSTALL)/usr/include/ztd
	cp -r include/* $(INSTALL)/usr/include/ztd

uninstall:
	rm $(INSTALL)/usr/lib/libztd.*
	rm -rd $(INSTALL)/usr/include/ztd

clean:
	rm $(ODIR)/*.o $(ODIR_SHARED)/*.o

clear:
	rm -rd libztd.a libztd.so doc
