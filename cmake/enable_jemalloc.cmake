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

set(ENABLE_JEMALLOC "OFF" CACHE STRING "Enable jemalloc memory allocator support: ON, OFF, AUTO")
set_property(CACHE ENABLE_JEMALLOC PROPERTY STRINGS AUTO ON OFF)
message(STATUS "Checking jemalloc support")

if(ENABLE_JEMALLOC STREQUAL "OFF")
  set(SYSLOG_NG_ENABLE_JEMALLOC OFF)
  message(STATUS "  jemalloc support: disabled (forced OFF)")
  return()
endif()

include(find_jemalloc)

if("${ENABLE_JEMALLOC}" MATCHES "^(auto|AUTO)$")
  if(JEMALLOC_FOUND)
    set(SYSLOG_NG_ENABLE_JEMALLOC ON)
    message(STATUS "  jemalloc support: enabled (AUTO, found libjemalloc)")
  else()
    set(SYSLOG_NG_ENABLE_JEMALLOC OFF)
    message(STATUS "  jemalloc support: disabled (AUTO, libjemalloc not found)")
  endif()
elseif(ENABLE_JEMALLOC STREQUAL "ON")
  if(NOT JEMALLOC_FOUND)
    message(FATAL_ERROR "Could not find libjemalloc, and jemalloc support was explicitly enabled.")
  endif()

  set(SYSLOG_NG_ENABLE_JEMALLOC ON)
  message(STATUS "  jemalloc support: enabled (forced ON)")
else()
  message(FATAL_ERROR "Invalid value (${ENABLE_JEMALLOC}) for ENABLE_JEMALLOC (must be ON, OFF, or AUTO)")
endif()
