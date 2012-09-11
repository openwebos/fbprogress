# @@@LICENSE
#
#      Copyright (c) 2007-2012 Hewlett-Packard Development Company, L.P.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# LICENSE@@@
cmake_minimum_required(VERSION 2.8.7)
project(fbprogress)
include(webOS/webOS)
webos_modules_init(1 0 0 QUALIFIER RC3)
webos_component(2 0 0)

webos_machine_dep()

webos_configure_header_files(src)

include_directories(src)
file(GLOB SOURCE_FILES src/*.cpp)
add_executable(fbprogress ${SOURCE_FILES})

target_link_libraries(fbprogress -lpthread)
webos_build_program()

if(${WEBOS_TARGET_MACHINE} STREQUAL "broadway")
	message(STATUS "Installing images for broadway device (small, no removable battery)")
	install(FILES files/share/320x50/0/common.tgz files/share/320x50/0/nobattery.tgz DESTINATION ${WEBOS_INSTALL_DATADIR}/fbprogress)
elseif(${WEBOS_TARGET_MACHINE} STREQUAL "opal")
	message(STATUS "Installing images for opal device (large, removable battery)")
	install(FILES files/share/480x75/0/common.tgz files/share/480x75/0/battery.tgz DESTINATION ${WEBOS_INSTALL_DATADIR}/fbprogress)
elseif(${WEBOS_TARGET_MACHINE} STREQUAL "topaz")
	message(STATUS "Installing images for topaz device (large, no removable battery, 270 degree rotation)")
	install(FILES files/share/480x75/270/common.tgz files/share/480x75/270/nobattery.tgz DESTINATION ${WEBOS_INSTALL_DATADIR}/fbprogress)
else()
	message(STATUS "Installing default images (small, no removable battery)")
	install(FILES files/share/320x50/0/common.tgz files/share/320x50/0/battery.tgz DESTINATION ${WEBOS_INSTALL_DATADIR}/fbprogress)
endif()
