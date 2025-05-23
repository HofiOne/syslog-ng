if(NOT ENABLE_CPP)
  return()
endif()

# C++ code

set(OTEL_LOGRECORD_CPP_SOURCES
  object-otel.h
  object-otel-logrecord.cpp
  object-otel-logrecord.hpp
  object-otel-resource.cpp
  object-otel-resource.hpp
  object-otel-scope.cpp
  object-otel-scope.hpp
  object-otel-kvlist.cpp
  object-otel-kvlist.hpp
  object-otel-array.cpp
  object-otel-array.hpp
  protobuf-field.cpp
  protobuf-field.hpp
  otel-field.hpp
  otel-field.cpp
)

add_module(
  TARGET otel_filterx_logrecord_cpp
  SOURCES ${OTEL_LOGRECORD_CPP_SOURCES}
  DEPENDS ${MODULE_GRPC_LIBS} grpc-protos
  INCLUDES ${OTEL_PROTO_BUILDDIR} ${PROJECT_SOURCE_DIR}/modules/grpc
  LIBRARY_TYPE STATIC
)

target_compile_options (otel_filterx_logrecord_cpp PRIVATE -Wno-double-promotion -Wno-deprecated -Wno-deprecated-declarations -DPROTOBUF_ENABLE_DEBUG_LOGGING_MAY_LEAK_PII=0)
