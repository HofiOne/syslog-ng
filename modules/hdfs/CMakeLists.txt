
module_switch(ENABLE_HDFS "Enable HDFS destination" TRUE)

if(ENABLE_HDFS)
  set(hdfs_SOURCES
    "hdfs-plugin.c"
    "hdfs-parser.h"
    "hdfs-parser.c"
    "hdfs-grammar.ym"

    # ${CMAKE_CURRENT_BINARY_DIR}/hdfs-grammar.c
    # ${CMAKE_CURRENT_BINARY_DIR}/hdfs-grammar.h
    "hdfs_destination.h"
    "hdfs_destination.c"
    "hdfs_destination_worker.h"
    "hdfs_destination_worker.c"
  )

  # generate_y_from_ym (modules/hdfs/hdfs-grammar)
  # bison_target (hdfsGrammar
  # ${CMAKE_CURRENT_BINARY_DIR}/hdfs-grammar.y
  # ${CMAKE_CURRENT_BINARY_DIR}/hdfs-grammar.c
  # COMPILE_FLAGS ${BISON_FLAGS})
  # set_source_files_properties (
  # ${CMAKE_CURRENT_BINARY_DIR}/hdfs-grammar.c
  # PROPERTIES
  # COMPILE_FLAGS ${BISON_BUILT_SOURCE_CFLAGS}
  # )

  # GRAMMAR param adds all the required grammar generation parts as well
  #
  add_module(
    TARGET hdfs
    GRAMMAR hdfs-grammar
    SOURCES ${hdfs_SOURCES}
  )

  FIND_PACKAGE(Boost 1.50 REQUIRED)
  FIND_PACKAGE(LibXml2 REQUIRED)
  FIND_PACKAGE(Protobuf REQUIRED)
  FIND_PACKAGE(KERBEROS REQUIRED)
  FIND_PACKAGE(GSasl REQUIRED)
  FIND_PACKAGE(OpenSSL REQUIRED)
  FIND_PACKAGE(CURL REQUIRED)

  # TODO: get these from the built depenedenccies output
  SET(LIBHDFS3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libhdfs3)

  SET (HDFS3_LIBRARIES ${LIBHDFS3_DIR}/lib/libhdfs3.so)
  #SET(HDFS3_LIBRARIES ${LIBHDFS3_DIR}/lib/libhdfs3.a)
  #set_target_properties(hdfs PROPERTIES COMPILE_FLAGS "-fPIC")

  INCLUDE_DIRECTORIES(${LIBHDFS3_DIR}/include)
  TARGET_LINK_DIRECTORIES(hdfs PUBLIC ${LIBHDFS3_DIR}/lib)
  TARGET_LINK_LIBRARIES(hdfs PUBLIC ${HDFS3_LIBRARIES})

  # FIXME: Does not work
  INSTALL(TARGETS hdfs LIBRARY DESTINATION lib/syslog-ng COMPONENT ${HDFS3_LIBRARIES})

  INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${KERBEROS_INCLUDE_DIRS})
  INCLUDE_DIRECTORIES(${GSASL_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})

  # TARGET_LINK_DIRECTORIES (hdfs PUBLIC ${Boost_LIBRARY_DIRS})
  TARGET_LINK_LIBRARIES(hdfs PUBLIC ${BOOST_LIBRARIES})
  TARGET_LINK_LIBRARIES(hdfs PUBLIC ${PROTOBUF_LIBRARIES})
  TARGET_LINK_LIBRARIES(hdfs PUBLIC ${LIBXML2_LIBRARIES})
  TARGET_LINK_LIBRARIES(hdfs PUBLIC ${KERBEROS_LIBRARIES})
  TARGET_LINK_LIBRARIES(hdfs PUBLIC ${GSASL_LIBRARIES})
  TARGET_LINK_LIBRARIES(hdfs PUBLIC ${OPENSSL_LIBRARIES})
  TARGET_LINK_LIBRARIES(hdfs PUBLIC ${CURL_LIBRARIES})
endif()
