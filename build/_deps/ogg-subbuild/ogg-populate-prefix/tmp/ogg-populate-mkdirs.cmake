# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/giang/OneDrive - UTS/Project/genetic-photo-recreation/build/_deps/ogg-src")
  file(MAKE_DIRECTORY "/Users/giang/OneDrive - UTS/Project/genetic-photo-recreation/build/_deps/ogg-src")
endif()
file(MAKE_DIRECTORY
  "/Users/giang/OneDrive - UTS/Project/genetic-photo-recreation/build/_deps/ogg-build"
  "/Users/giang/Library/CloudStorage/OneDrive-UTS/Project/genetic-photo-recreation/build/_deps/ogg-subbuild/ogg-populate-prefix"
  "/Users/giang/Library/CloudStorage/OneDrive-UTS/Project/genetic-photo-recreation/build/_deps/ogg-subbuild/ogg-populate-prefix/tmp"
  "/Users/giang/Library/CloudStorage/OneDrive-UTS/Project/genetic-photo-recreation/build/_deps/ogg-subbuild/ogg-populate-prefix/src/ogg-populate-stamp"
  "/Users/giang/Library/CloudStorage/OneDrive-UTS/Project/genetic-photo-recreation/build/_deps/ogg-subbuild/ogg-populate-prefix/src"
  "/Users/giang/Library/CloudStorage/OneDrive-UTS/Project/genetic-photo-recreation/build/_deps/ogg-subbuild/ogg-populate-prefix/src/ogg-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/giang/Library/CloudStorage/OneDrive-UTS/Project/genetic-photo-recreation/build/_deps/ogg-subbuild/ogg-populate-prefix/src/ogg-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/giang/Library/CloudStorage/OneDrive-UTS/Project/genetic-photo-recreation/build/_deps/ogg-subbuild/ogg-populate-prefix/src/ogg-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
