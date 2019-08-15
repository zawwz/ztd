IDIR=include
SRCDIR=src
ODIR=obj
ODIR_SHARED=obj_so

NAME = libztd

CC=g++
CXXFLAGS= -I$(IDIR) -Wall -pedantic -std=c++17
ifeq	 ($(DEBUG),true)
	CXXFLAGS += -g
endif

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

install: all
	mv libztd.a libztd.so /usr/local/lib
	cp -r include /usr/local/include/ztd

uninstall:
	rm /usr/local/lib/libztd.*
	rm -rd /usr/local/include/ztd

clean:
	rm $(ODIR)/*.o $(ODIR_SHARED)/*.o
