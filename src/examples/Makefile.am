AM_CXXFLAGS = $(WARNING_CXXFLAGS) $(PICKY_CXXFLAGS) $(HARDEN_CFLAGS) $(MISC_CXXFLAGS)
AM_LDFLAGS  = $(HARDEN_LDFLAGS)

if BUILD_EXAMPLES
  noinst_PROGRAMS = ntester cellproxy cellsim sproutbt2
endif

ntester_SOURCES = ntester.cc
ntester_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../statesync -I$(srcdir)/../terminal -I$(srcdir)/../network -I$(srcdir)/../crypto -I$(srcdir)/../sprout -I../protobufs $(protobuf_CFLAGS)
ntester_LDADD = ../statesync/libmoshstatesync.a ../network/libmoshnetwork.a ../sprout/libsprout.a ../crypto/libmoshcrypto.a ../protobufs/libmoshprotos.a ../util/libmoshutil.a $(LIBUTIL) -lm $(protobuf_LIBS)  $(OPENSSL_LIBS)

sproutbt2_SOURCES = sproutbt2.cc
sproutbt2_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../network -I$(srcdir)/../crypto -I$(srcdir)/../sprout -I../protobufs $(protobuf_CFLAGS)
sproutbt2_LDADD = ../network/libmoshnetwork.a ../sprout/libsprout.a ../crypto/libmoshcrypto.a ../protobufs/libmoshprotos.a ../util/libmoshutil.a $(LIBUTIL) -lm $(protobuf_LIBS)  $(OPENSSL_LIBS)

cellproxy_SOURCES = cellproxy.cc
cellproxy_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../network -I$(srcdir)/../crypto -I$(srcdir)/../sprout -I../protobufs $(protobuf_CFLAGS)
cellproxy_LDADD = ../network/libmoshnetwork.a ../sprout/libsprout.a ../crypto/libmoshcrypto.a ../protobufs/libmoshprotos.a ../util/libmoshutil.a $(LIBUTIL) -lm $(protobuf_LIBS)  $(OPENSSL_LIBS)

cellsim_SOURCES = cellsim.cc
cellsim_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../network -I$(srcdir)/../crypto -I$(srcdir)/../sprout -I../protobufs $(protobuf_CFLAGS)
cellsim_LDADD = ../network/libmoshnetwork.a ../sprout/libsprout.a ../crypto/libmoshcrypto.a ../protobufs/libmoshprotos.a ../util/libmoshutil.a $(LIBUTIL) -lm $(protobuf_LIBS)  $(OPENSSL_LIBS)
