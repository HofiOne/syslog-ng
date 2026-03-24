# ############################################################################
# Copyright (c) 2025 Istvan Hoffmann <hofione@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
# As an additional exemption you are allowed to compile & link against the
# OpenSSL libraries as published by the OpenSSL project. See the file
# COPYING for details.
#
# ############################################################################

# Find jemalloc library
#
# On FreeBSD, jemalloc is built into libc, so no separate library is needed.
# On Linux, jemalloc is a separate library.
#
# Sets the following variables:
# JEMALLOC_FOUND        - True if jemalloc found
# JEMALLOC_INCLUDE_DIRS - where to find jemalloc headers
# JEMALLOC_LIBRARIES    - List of libraries when using jemalloc (empty on FreeBSD)

include(FindPackageHandleStandardArgs)
include(CheckIncludeFile)

message(STATUS "  Searching for jemalloc...")
message(STATUS "    CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")

# Check if we're on FreeBSD where jemalloc is built into libc
if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  message(STATUS "    Detected FreeBSD - checking for built-in jemalloc")

  # On FreeBSD, jemalloc functions are in stdlib.h and malloc_np.h
  check_include_file(malloc_np.h HAVE_MALLOC_NP_H)

  if(HAVE_MALLOC_NP_H)
    set(JEMALLOC_FOUND TRUE)
    set(JEMALLOC_INCLUDE_DIR "")
    set(JEMALLOC_INCLUDE_DIRS "")
    set(JEMALLOC_LIBRARY "")
    set(JEMALLOC_LIBRARIES "")
    message(STATUS "    FreeBSD built-in jemalloc found (part of libc)")
  else()
    set(JEMALLOC_FOUND FALSE)
    message(STATUS "    FreeBSD built-in jemalloc not found")
  endif()

else() # On other systems, look for jemalloc as a separate library
  # Look for the header file
  find_path(JEMALLOC_INCLUDE_DIR
    NAMES jemalloc/jemalloc.h
    HINTS
    ENV JEMALLOC_ROOT
    ${JEMALLOC_ROOT}
  )
  mark_as_advanced(JEMALLOC_INCLUDE_DIR)

  message(STATUS "    JEMALLOC_INCLUDE_DIR: ${JEMALLOC_INCLUDE_DIR}")

  # Look for the library
  find_library(JEMALLOC_LIBRARY
    NAMES jemalloc
    HINTS
    ENV JEMALLOC_ROOT
    ${JEMALLOC_ROOT}
  )
  mark_as_advanced(JEMALLOC_LIBRARY)

  message(STATUS "    JEMALLOC_LIBRARY: ${JEMALLOC_LIBRARY}")

  find_package_handle_standard_args(jemalloc REQUIRED_VARS JEMALLOC_LIBRARY JEMALLOC_INCLUDE_DIR)

  # Copy the result to output variables
  if(JEMALLOC_FOUND)
    set(JEMALLOC_LIBRARIES ${JEMALLOC_LIBRARY})
    set(JEMALLOC_INCLUDE_DIRS ${JEMALLOC_INCLUDE_DIR})
  endif()
endif()

# Clear variables if not found
if(NOT JEMALLOC_FOUND)
  set(JEMALLOC_LIBRARY)
  set(JEMALLOC_LIBRARIES)
  set(JEMALLOC_INCLUDE_DIR)
  set(JEMALLOC_INCLUDE_DIRS)
endif()
