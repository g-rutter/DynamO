cmake_minimum_required (VERSION 2.6 FATAL_ERROR)
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to Release")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build, options are: Debug Release" FORCE)
endif()

##########   OPTIONS
project(DynamO) #Project name
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

enable_testing() #Enable build of test executables and 'make test' command
include(CTest)

######################################################################
########## Packaging
######################################################################
include(SystemInformation)
include(GetGitRevisionDescription)

git_describe(GIT_RESULT --match initial-commit)
message(STATUS "Git description is ${GIT_RESULT}")
string(REGEX MATCH "[0-9]+" BUILD_NUMBER ${GIT_RESULT})
message(STATUS "Build number is ${BUILD_NUMBER}")
set(CPACK_GENERATOR ${SPECIFIC_SYSTEM_PREFERED_CPACK_GENERATOR})
set(CPACK_PACKAGE_NAME "dynamomd")
set(CPACK_PACKAGE_VERSION_MAJOR 1)
set(CPACK_PACKAGE_VERSION_MINOR 6)
set(CPACK_PACKAGE_VERSION_PATCH ${BUILD_NUMBER})
set(CPACK_PACKAGE_CONTACT "support@dynamomd.org")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "An event-driven particle simulation tool.")
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}~${DISTRO_ID}${DISTRO_RELEASE})
#Get Debian packages to automatically figure out the dependencies
set (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
include(CPack) #Enable build of packages

##########   RELEASE MODE
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")

##########   DEBUG MODE
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDYNAMO_DEBUG -DMAGNET_DEBUG -Wall -g")

link_libraries(debug dl)

######################################################################
########## COMPILER C++11/C++0x SUPPORT TESTS
######################################################################
### First check if the compiler supports C++11 or C++0x at all
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORT_CXX11)
if(COMPILER_SUPPORT_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
  check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORT_CXX0X)
  if(COMPILER_SUPPORT_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    check_cxx_source_compiles("int main(int, const char**) {int array[5] = {1,2,3,4,5}; for(int& x: array) x *= 2;}" CXX0X_RANGE_BASED_FOR)
    if(NOT CXX0X_RANGE_BASED_FOR)
      message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has limited C++11 support. You must install a more modern C++ compiler.")
    endif()
  else()
    message(SEND_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. You must install a more modern C++ compiler.")
  endif()
endif()

### Now check if the C++11/0x support is sufficient for the compile time math library
set(CMAKE_REQUIRED_FLAGS -Wfatal-errors)
check_cxx_source_compiles("template <typename T> struct wrapped{ typedef T type;};
template <typename T> typename T::type unwrap1(T a) { return typename T::type();}
int unwrap(int a) { return 1; }
template <typename T> auto unwrap(T t) -> decltype(unwrap(unwrap1(t))) { return unwrap(unwrap1(t)); }
int main() { unwrap(wrapped<wrapped<int>>()); }" CXX11_TYPE_MANGLING)
if(NOT CXX11_TYPE_MANGLING)
  message(WARNING "Although the compiler advertises support for C++0x/C++11, this support is limited (e.g., gcc-4.7 or lower). You can only compile a limited version of DynamO (simple potentials only).")
  else()
    set(CMAKE_REQUIRED_FLAGS -Wfatal-errors)
    set(CMAKE_REQUIRED_INCLUDES ${CMAKE_SOURCE_DIR}/src/magnet ${CMAKE_SOURCE_DIR}/src/ext_include)
    check_cxx_source_compiles("#include <magnet/math/symbolic.hpp>
    int main() { using namespace magnet::math; Variable<'x'> x; simplify(x+(x-1)); }" CXX11_decltype_works)
    if(NOT CXX11_decltype_works)
      message(WARNING "Although the compiler advertises support for C++0x/11, this support is broken (e.g., gcc-4.8.2). You can only compile a limited version of DynamO (simple potentials only).")
    endif()  
endif()
unset(CMAKE_REQUIRED_FLAGS)
unset(CMAKE_REQUIRED_INCLUDES)

######################################################################
##########  Boost support
######################################################################
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.46 COMPONENTS "program_options" "filesystem" "iostreams" "system" "unit_test_framework")
if(NOT Boost_FOUND)
  message(STATUS "Cannot find a static installation of boost, trying a dynamic version")
  set(Boost_USE_STATIC_LIBS OFF)
  find_package(Boost 1.46 REQUIRED COMPONENTS "program_options" "filesystem" "iostreams" "system" "unit_test_framework")
  if(NOT Boost_FOUND)
    message(SEND_ERROR "Cannot find a static installation of boost, trying a dynamic version")
  endif()
endif()
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${Boost_FILESYSTEM_LIBRARY} ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Boost_IOSTREAMS_LIBRARY} ${Boost_SYSTEM_LIBRARY})

#BZip2 support required for boost iostreams
find_package(BZip2 REQUIRED)
include_directories(${BZIP2_INCLUDE_DIR})
link_libraries(${BZIP2_LIBRARIES})

######################################################################
# Determine the supported system thread library and link to it
######################################################################
find_package(Threads)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

######################################################################
# Test for libjudy requirements
######################################################################
include(CheckIncludeFiles)
check_include_files(Judy.h JUDY_H_AVAILABLE)
include(CheckLibraryExists)
check_library_exists(Judy  Judy1FreeArray "" JUDY_LIB_AVAILABLE)
include(CheckTypeSize)
check_type_size(size_t SIZEOF_SIZE_T)
set(LIBJUDY_SUPPORT FALSE)
if(SIZEOF_SIZE_T LESS 8)
  message(STATUS "libJudy support disabled (no 64 bit support).")
elseif(JUDY_H_AVAILABLE AND JUDY_LIB_AVAILABLE) 
  message(STATUS "Enabling libJudy support.")
  set(LIBJUDY_SUPPORT TRUE)
  add_definitions(-DDYNAMO_JUDY)
  link_libraries(Judy)
else()
  message(STATUS "libJudy header/library missing.")
endif()

######################################################################
# Visualiser support
######################################################################
INCLUDE (CheckIncludeFiles)
find_package(PkgConfig)
find_package(OpenGL)
find_package(GLUT)
find_package(GLEW)
find_package(AVCodec)
pkg_check_modules(GTKMM gtkmm-2.4)
pkg_check_modules(CAIROMM cairomm-1.0)
pkg_check_modules(PNG libpng)

set(VISUALIZER_SUPPORT TRUE)
function(visualiser_dependency varname message)
  if(${varname})
    message(STATUS "${message} found.")
  else()
    message(STATUS "${message} not found - visualiser will not be built.")
    set(VISUALIZER_SUPPORT FALSE PARENT_SCOPE)
  endif()
endfunction(visualiser_dependency)

visualiser_dependency(OPENGL_FOUND "OpenGL libraries")
visualiser_dependency(GTKMM_FOUND "Gtkmm library")
visualiser_dependency(PNG_FOUND "libPNG library")
visualiser_dependency(GLUT_FOUND "GLUT/freeGLUT library")
visualiser_dependency(GLEW_FOUND "GLEW library")
visualiser_dependency(CAIROMM_FOUND "Cairomm library")

if(VISUALIZER_SUPPORT)
  check_include_files(cwiid.h CWIID_H_AVAILABLE)
  check_library_exists(cwiid  cwiid_open "" CWIID_LIB_AVAILABLE)
  if(CWIID_H_AVAILABLE AND CWIID_LIB_AVAILABLE)
    message(STATUS "Enabling Wiimote head tracking support")
    add_definitions(-DCOIL_wiimote)
    link_libraries(cwiid)
  endif()
  
  add_definitions(${OPENGL_DEFINITONS} ${GLUT_DEFINITIONS} ${GLEW_DEFINITIONS})
  include_directories(${GTKMM_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS} ${CAIROMM_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIRS} ${GLUT_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS})
  link_libraries(${GTKMM_LIBRARIES} ${CAIROMM_LIBRARIES} ${PNG_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${GLEW_LIBRARIES})
  
  find_file(NEW_CAIRO_CONFIG_FOUND "cairommconfig.h" PATHS ${CAIROMM_INCLUDE_DIRS})
  if(NOT NEW_CAIRO_CONFIG_FOUND)
    message("Trying cairomm/ for cairommconfig.h")
    find_file(OLD_CAIRO_CONFIG_FOUND "cairomm/cairommconfig.h" PATHS ${CAIROMM_INCLUDE_DIRS})
    if(NOT OLD_CAIRO_CONFIG_FOUND)
      message(SEND_ERROR "Failed to find cairommconfig.h")
    endif()
    add_definitions(-DOLD_CAIRO_CONFIG)
  else()
    message("Located cairommconfig.h")
  endif()
endif()

######################################################################
########## HEADER DIRECTORIES
######################################################################
include_directories(${PROJECT_SOURCE_DIR}/src/dynamo/)
include_directories(${PROJECT_SOURCE_DIR}/src/coil/)
include_directories(${PROJECT_SOURCE_DIR}/src/magnet)
include_directories(${PROJECT_SOURCE_DIR}/src/ext_include)

######################################################################
######### TARGETS
######################################################################
### Magnet library
function(magnet_test name) #Registers a magnet unit-test
  add_executable(magnet_${name}_exe ${CMAKE_CURRENT_SOURCE_DIR}/src/magnet/tests/${name}.cpp)
  target_link_libraries(magnet_${name}_exe)
  add_test(magnet_${name} magnet_${name}_exe)
endfunction(magnet_test)

#magnet_test(scan_test : tests/scan_test.cpp magnet /opencl//OpenCL)
#magnet_test(radixsort_NVIDIA_test : tests/radixsort_NVIDIA_test.cpp magnet /opencl//OpenCL scan_test ;
#magnet_test(radixsort_AMD_test : tests/radixsort_AMD_test.cpp magnet /opencl//OpenCL scan_test ;
#magnet_test(bitonicsort_test : tests/bitonicsort_test.cpp magnet /opencl//OpenCL ;
#magnet_test(heapsort_test : tests/heapsort_test.cpp magnet /opencl//OpenCL ;
#magnet_test(sorter_test : tests/sorter_test.cpp magnet /opencl//OpenCL ;

magnet_test(threadpool_test)
SET_TARGET_PROPERTIES(magnet_threadpool_test_exe PROPERTIES LINK_FLAGS -Wl,--no-as-needed) #Fix for a bug in gcc

target_link_libraries(magnet_threadpool_test_exe ${CMAKE_THREAD_LIBS_INIT})
magnet_test(cubic_quartic_test)
magnet_test(vector_test)
magnet_test(quaternion_test)
magnet_test(dilate_test)
magnet_test(splinetest)
magnet_test(plane_intersection)
magnet_test(triangle_intersection)
magnet_test(intersection_genalg)
magnet_test(offcenterspheres)
magnet_test(stack_vector_test)

#### CAS library tests ####
if(CXX11_TYPE_MANGLING AND CXX11_decltype_works)
  magnet_test(polynomial_test)
  magnet_test(symbolic_test)
  magnet_test(stable_alg_test)
  magnet_test(numeric_test)
  magnet_test(symbolic_performance)  
endif()

if(JUDY_SUPPORT)
  magnet_test(judy_test)
endif(JUDY_SUPPORT)

if(AVCodec_FOUND)
  set(CMAKE_REQUIRED_INCLUDES ${AVCodec_INCLUDE_DIRS})
  check_cxx_source_compiles("#include <libavcodec/avcodec.h>
int main() { return AV_CODEC_ID_H264; }" AVCodec_New_Codec_Style)
  if(${AVCodec_New_Codec_Style})
    message(STATUS "Found suitable libAVCodec version")
    link_libraries(${AVCodec_LIBRARIES})
    include_directories(${AVCodec_INCLUDE_DIRS})
    add_definitions(-DMAGNET_FFMPEG_SUPPORT)
  else()
    message(STATUS "libAVCodec version is out of date or incompatible - recording will be disabled")
  endif()
endif()

### Coil
# configuration
if(VISUALIZER_SUPPORT)
  message(STATUS "Visualiser dependencies satisfied.")
  
  # Coil library
  file(GLOB_RECURSE coil_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/coil/coil/*.cpp)
  add_library(coil STATIC ${coil_SRC})
  link_libraries(coil)
  add_definitions(-DDYNAMO_visualizer)
  add_executable(coilvol ${CMAKE_CURRENT_SOURCE_DIR}/src/coil/programs/coilvol.cpp)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/coilvol DESTINATION bin)
endif()

### DynamO
file(GLOB_RECURSE dynamo_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/dynamo/*.cpp)
add_library(dynamo STATIC ${dynamo_SRC})
link_libraries(dynamo)

function(dynamo_exe name) #Registers a dynamo executable given the source file name
  add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/programs/${name}.cpp)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${name} DESTINATION bin)
endfunction(dynamo_exe)
dynamo_exe(dynarun)
dynamo_exe(dynamod)
dynamo_exe(dynahist_rw)
dynamo_exe(dynapotential)
#dynamo_exe(dynacollide)
if(VISUALIZER_SUPPORT)
  add_executable(dynavis ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/programs/dynarun.cpp)
  set_target_properties(dynavis PROPERTIES COMPILE_DEFINITIONS "DYNAMO_loadvisualiser")
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/dynavis DESTINATION bin)
endif()

# scripts
find_package(PythonInterp 2.7)
if(PYTHONINTERP_FOUND)
  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/programs/dynatransport DESTINATION bin)
  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/programs/dynamo2xyz    DESTINATION bin)
  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/programs/dynarmsd      DESTINATION bin)
  install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/programs/dynamaprmsd   DESTINATION bin)
else()
  message(WARNING "Python 2.7 not found, cannot install all DynamO tools.")
endif()

# unit tests
function(dynamo_test name) #Registers a unit test of DynamO
  add_executable(dynamo_${name}_exe ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/tests/${name}.cpp)
  set_target_properties(dynamo_${name}_exe PROPERTIES COMPILE_DEFINITIONS "BOOST_TEST_DYN_LINK")
  add_test(dynamo_${name} dynamo_${name}_exe)
endfunction(dynamo_test)

dynamo_test(hardsphere_test)
dynamo_test(shearing_test)
dynamo_test(binaryhardsphere_test)
dynamo_test(squarewell_test)
dynamo_test(2dstepped_potential_test)
dynamo_test(infmass_spheres_test)
dynamo_test(lines_test)
dynamo_test(static_spheres_test)
dynamo_test(gravityplate_test)
dynamo_test(polymer_test)
dynamo_test(swingspheres_test)
dynamo_test(squarewellwall_test)
dynamo_test(thermalisedwalls_test)
dynamo_test(event_sorters_test)

add_test(NAME dynamo_replica_exchange
         COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/src/dynamo/tests/replex_test.py 
	 --dynarun=$<TARGET_FILE:dynarun>
	 --dynamod=$<TARGET_FILE:dynamod>
	 --dynahist_rw=$<TARGET_FILE:dynahist_rw>)