libtestdir			    =	\
	$(pkglibdir)/libtest
libtestincludedir		    =	\
	$(pkgincludedir)/libtest

EXTRA_DIST += libtest/CMakeLists.txt

libtest_LIBRARIES		    =	\
	libtest/libsyslog-ng-test.a

libtest_libsyslog_ng_test_a_SOURCES =   \
	libtest/config_parse_lib.c	\
	libtest/config_parse_lib.h	\
	libtest/cr_template.c		\
	libtest/grab-logging.c		\
	libtest/grab-logging.h		\
	libtest/libtest.c		\
	libtest/mock-cfg-parser.c	\
	libtest/mock-cfg-parser.h	\
	libtest/mock-logpipe.c		\
	libtest/mock-logpipe.h		\
	libtest/mock-transport.c	\
	libtest/mock-transport.h	\
	libtest/fake-time.c		\
	libtest/fake-time.h		\
	libtest/msg_parse_lib.c		\
	libtest/msg_parse_lib.h		\
	libtest/persist_lib.c		\
	libtest/persist_lib.h		\
	libtest/proto_lib.c		\
	libtest/proto_lib.h		\
	libtest/queue_utils_lib.c	\
	libtest/queue_utils_lib.h	\
	libtest/stopwatch.c		\
	libtest/stopwatch.h

libtest_libsyslog_ng_test_a_CFLAGS = $(AM_CFLAGS) $(CRITERION_CFLAGS) $(JSON_CFLAGS)

libtestinclude_HEADERS		    =	\
	libtest/config_parse_lib.h		\
	libtest/cr_template.h		\
	libtest/fake-time.h		\
	libtest/grab-logging.h		\
	libtest/mock-cfg-parser.h		\
	libtest/mock-transport.h	\
	libtest/msg_parse_lib.h		\
	libtest/persist_lib.h		\
	libtest/proto_lib.h		\
	libtest/queue_utils_lib.h		\
	libtest/stopwatch.h

pkgconfig_DATA			   +=	\
	libtest/syslog-ng-test.pc

LIBTEST_LIBS = $(top_builddir)/libtest/libsyslog-ng-test.a $(JSON_LIBS)

include libtest/tests/Makefile.am
