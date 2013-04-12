# Copyright 2013 Dario Lombardo <lomato@gmail.com>
#
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.6)
project(tentacool CXX)
set(hcrt-dissector_VERSION_MAJOR 0)
set(hcrt-dissector_VERSION_MINOR 0)
enable_testing()

########################################################################
# Setup the include and linker paths
########################################################################
include_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "")

# Set component parameters
set(TENTACOOL_INCLUDE_DIRS ${CMAKE_SOURCE_DIR} CACHE INTERNAL "" FORCE)

set(TENTACOOL_SRC 
  ${CMAKE_SOURCE_DIR}/main.cpp
)

add_library(tentacool
    MODULE
    ${TENTACOOL_SRC}
)

set_target_properties(tentacool PROPERTIES PREFIX "")
set_target_properties(tentacool PROPERTIES LINK_FLAGS "-lPocoFoundation")
target_link_libraries(tentacool "")
