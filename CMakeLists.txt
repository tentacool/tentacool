#
# Copyright 2013
# Dario Lombardo <lomato@gmail.com>
# Sebastiano Di Paola <sebastiano.dipaola@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#

########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.6)
project(tentacool CXX)
set(tentacool_VERSION_MAJOR 0)
set(tentacool_VERSION_MINOR 1)
enable_testing()

########################################################################
# Setup the include and linker paths
########################################################################
include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

set(TENTACOOL_LINK_FLAGS
    "-lPocoFoundation -lPocoNet")

# Set component parameters
set(TENTACOOL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR} CACHE INTERNAL "" FORCE)

set(TENTACOOL_SRC
  ${CMAKE_SOURCE_DIR}/main.cpp
  ${CMAKE_SOURCE_DIR}/hpfeedclient.cpp
  ${CMAKE_SOURCE_DIR}/hpfeedbroker.cpp
  ${CMAKE_SOURCE_DIR}/authenticator.cpp
  ${CMAKE_SOURCE_DIR}/message_router.cpp
)

#######################################################################
# Setup the CFLAGS (for any environment)
#######################################################################
set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG")

add_executable(tentacool ${TENTACOOL_SRC})

set_target_properties(tentacool PROPERTIES PREFIX "")
target_link_libraries(tentacool "${TENTACOOL_LINK_FLAGS}")
