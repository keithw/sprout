AM_CXXFLAGS = $(WARNING_CXXFLAGS) $(PICKY_CXXFLAGS) $(HARDEN_CFLAGS) $(MISC_CXXFLAGS)
AM_LDFLAGS  = $(HARDEN_LDFLAGS)

if BUILD_EXAMPLES
  noinst_PROGRAMS = encrypt decrypt ntester parse termemu cellproxy cellsim sproutbt2
endif

encrypt_SOURCES = encrypt.cc
encrypt_CPPFLAGS = -I$(srcdir)/../crypto
encrypt_LDADD = ../crypto/libmoshcrypto.a $(OPENSSL_LIBS)

decrypt_SOURCES = decrypt.cc
decrypt_CPPFLAGS = -I$(srcdir)/../crypto
decrypt_LDADD = ../crypto/libmoshcrypto.a $(OPENSSL_LIBS)

parse_SOURCES = parse.cc
parse_CPPFLAGS = -I$(srcdir)/../terminal -I$(srcdir)/../util
parse_LDADD = ../terminal/libmoshterminal.a ../util/libmoshutil.a $(LIBUTIL)

termemu_SOURCES = termemu.cc
termemu_CPPFLAGS = -I$(srcdir)/../terminal -I$(srcdir)/../util -I$(srcdir)/../statesync -I../protobufs
termemu_LDADD = ../terminal/libmoshterminal.a ../util/libmoshutil.a ../statesync/libmoshstatesync.a ../protobufs/libmoshprotos.a $(LIBUTIL) $(TINFO_LIBS) $(protobuf_LIBS)

ntester_SOURCES = ntester.cc
ntester_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../statesync -I$(srcdir)/../terminal -I$(srcdir)/../network -I$(srcdir)/../crypto -I$(srcdir)/../sprout -I../protobufs $(protobuf_CFLAGS)
ntester_LDADD = ../statesync/libmoshstatesync.a ../terminal/libmoshterminal.a ../network/libmoshnetwork.a ../sprout/libsprout.a ../crypto/libmoshcrypto.a ../protobufs/libmoshprotos.a ../util/libmoshutil.a $(LIBUTIL) -lm $(protobuf_LIBS)  $(OPENSSL_LIBS)

sproutbt2_SOURCES = sproutbt2.cc
sproutbt2_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../network -I$(srcdir)/../crypto -I$(srcdir)/../sprout -I../protobufs $(protobuf_CFLAGS)
sproutbt2_LDADD = ../network/libmoshnetwork.a ../sprout/libsprout.a ../crypto/libmoshcrypto.a ../protobufs/libmoshprotos.a ../util/libmoshutil.a $(LIBUTIL) -lm $(protobuf_LIBS)  $(OPENSSL_LIBS)

cellproxy_SOURCES = cellproxy.cc
cellproxy_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../network -I$(srcdir)/../crypto -I$(srcdir)/../sprout -I../protobufs $(protobuf_CFLAGS)
cellproxy_LDADD = ../network/libmoshnetwork.a ../sprout/libsprout.a ../crypto/libmoshcrypto.a ../protobufs/libmoshprotos.a ../util/libmoshutil.a $(LIBUTIL) -lm $(protobuf_LIBS)  $(OPENSSL_LIBS)

cellsim_SOURCES = cellsim.cc
cellsim_CPPFLAGS = -I$(srcdir)/../util -I$(srcdir)/../network -I$(srcdir)/../crypto -I$(srcdir)/../sprout -I../protobufs $(protobuf_CFLAGS)
cellsim_LDADD = ../network/libmoshnetwork.a ../sprout/libsprout.a ../crypto/libmoshcrypto.a ../protobufs/libmoshprotos.a ../util/libmoshutil.a $(LIBUTIL) -lm $(protobuf_LIBS)  $(OPENSSL_LIBS)
