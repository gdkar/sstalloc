CFLAGS += -g -ggdb  -O3 -Wstrict-prototypes -fomit-frame-pointer  -Wall -std=c11 -D_GNU_SOURCE=1 -D_ISOC11_SOURCE=1
libdir = /usr/lib
LDFLAGS += -lpthread -rpath $(libdir) -version-info 1
CC = gcc
CXX = gcc
LD = ld
ARCH = $(shell uname -m)

# Check Architecture
SUPPORTED_ARCH = NO

ifeq ($(ARCH), x86_64)
SUPPORTED_ARCH = YES
endif

ifeq ($(SUPPORTED_ARCH), NO)
$(error Your architecture $(ARCH) is not currently supported. See README.)
endif

define compile_rule
	libtool --mode=compile --tag=CC \
	$(CC) $(CFLAGS) $(CPPFLAGS) -Ilib/replace -Iinclude -Iinclude-$(ARCH) -c $<
endef

define cxx_compile_rule
	libtool --mode=compile --tag=CC \
	$(CXX) $(CFLAGS) $(CPPFLAGS) -Ilib/replace -Iinclude -Iinclude-$(ARCH) -c $<
endef

define link_rule
	libtool --mode=link --tag=CC \
	$(LD) $(LDFLAGS) -o $@ $^ $(LDLIBS)
endef

LIBS = libstalloc.la
libstalloc_OBJS = src/stalloc.lo src/stack.lo #new_delete.lo

%.lo: %.c
	$(call compile_rule)
%.lo: %.cpp
	$(call cxx_compile_rule)

all: libstalloc.la cmxchg cmxcghq

libstalloc.la: $(libstalloc_OBJS)
	$(call link_rule)
	cp .libs/libstalloc.so ./
	cp .libs/libstalloc.a ./

install/%.la: %.la
	libtool --mode=install \
	install -c $(notdir $@) $(libdir)/$(notdir $@)

install: $(addprefix install/,$(LIBS))
	libtool --mode=finish $(libdir)

clean:
	libtool --mode=clean rm *.la *.lo *.a *.so -f

