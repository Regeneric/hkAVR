# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/hkk/Documents/projects/hkAVRm/pico-sdk/tools/pioasm")
  file(MAKE_DIRECTORY "/home/hkk/Documents/projects/hkAVRm/pico-sdk/tools/pioasm")
endif()
file(MAKE_DIRECTORY
  "/home/hkk/Documents/projects/hkAVRm/cmake-build/pioasm"
  "/home/hkk/Documents/projects/hkAVRm/cmake-build/pioasm-install"
  "/home/hkk/Documents/projects/hkAVRm/cmake-build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/tmp"
  "/home/hkk/Documents/projects/hkAVRm/cmake-build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
  "/home/hkk/Documents/projects/hkAVRm/cmake-build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src"
  "/home/hkk/Documents/projects/hkAVRm/cmake-build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/hkk/Documents/projects/hkAVRm/cmake-build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/hkk/Documents/projects/hkAVRm/cmake-build/pico-sdk/src/rp2_common/pico_cyw43_driver/pioasm/src/pioasmBuild-stamp${cfgdir}") # cfgdir has leading slash
endif()
