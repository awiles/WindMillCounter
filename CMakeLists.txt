project(WindMillCounter)

cmake_minimum_required(VERSION 2.8.11)

# if we don't define it, let's assume debug.
if(NOT DEFINED CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# Output directories.
SET(LIBRARY_OUTPUT_PATH ${WindMillCounter_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all libraries.")
SET(EXECUTABLE_OUTPUT_PATH ${WindMillCounter_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
SET(CXX_TEST_PATH ${EXECUTABLE_OUTPUT_PATH})

# initialize the prefix path.
set(CMAKE_PREFIX_PATH)
# add the directory above to help search for related projects (e.g. openExternals)
list(APPEND CMAKE_PREFIX_PATH ${WindMillCounter_SOURCE_DIR}/..)

# set up search paths for cmake modules.
set(CMAKE_MODULE_PATH ${WindMillCounter_SOURCE_DIR}/cmake)

# set the QT_DIR and set up some other variables.
if(WIN32)
	# TODO:This works for Windows, MSVC 2012, 64 bits.  Need logic here based on the generator.
	# NOTE: this may not be the best way to handle this -- need to figure out the best path forward here.
    set(QT_DIR "C:\\Qt\\5.2.1\\msvc2012_64_opengl" CACHE PATH "Need to set the Qt prefix for various things.")
    set(CMAKE_LIBRARY_PATH "C:\\Program Files (x86)\\Windows Kits\\8.0\\Lib\\win8\\um\\x64" CACHE PATH "Qt5 requires this.")
	# Need wzunzip.exe for Boost download on windows.
elseif(APPLE)
    # assume QT is installed to /opt/Qt
    # need to use 5.4 with VTK 6.3 on Mac OSX Sierra.
    set(QT_DIR "/opt/Qt/5.4/clang_64" CACHE PATH "Need to set the Qt prefix for various things.")
else()
    set(QT_DIR "/opt/Qt/5.2.1/gcc_64" CACHE PATH "Need to set the Qt path in order to find it more easily." )
endif()
# update the prefixes.
list(APPEND CMAKE_PREFIX_PATH ${QT_DIR})

# Looking for openExternals
find_package(openExternals)
if(openExternals_FOUND)
	message(STATUS "Found openExternals: ${openExternals_DIR}")
	list( APPEND CMAKE_PREFIX_PATH ${openExternals_DIR} )
			
	#eigen
	set(WindMillCounter_EIGEN_DIR ${openExternals_EIGEN_DIR})
	list( APPEND CMAKE_PREFIX_PATH ${WindMillCounter_EIGEN_DIR} )
	
	#vtk
	set(WindMillCounter_VTK_DIR ${openExternals_VTK_DIR})
	list( APPEND CMAKE_PREFIX_PATH ${WindMillCounter_VTK_DIR} )

	#opencv
	set(WindMillCounter_OPENCV_DIR ${openExternals_OPENCV_DIR})
	list( APPEND CMAKE_PREFIX_PATH ${WindMillCounter_OPENCV_DIR} )
	
	#opencv
	set(WindMillCounter_OPENIGTLINK_DIR ${openExternals_OPENIGTLINK_DIR})
	list( APPEND CMAKE_PREFIX_PATH ${WindMillCounter_OPENIGTLINK_DIR} )
		
	
	# #boost
	# set(WindMillCounter_BOOST_DIR ${openExternals_BOOST_DIR})
	# message(STATUS "Boost dir: ${WindMillCounter_BOOST_DIR}")
	# set(BOOST_ROOT ${WindMillCounter_BOOST_DIR} )
	# set(BOOST_INCLUDEDIR ${BOOST_ROOT}/include )
	# set(BOOST_LIBRARYDIR ${BOOST_ROOT}/lib )
	# set(Boost_NO_SYSTEM_PATHS ON )
	# set(Boost_USE_STATIC_LIBS ON )
	
	# #flann
	# set(WindMillCounter_FLANN_INCLUDE_DIR ${openExternals_FLANN_INCLUDE_DIR})
	# set(WindMillCounter_FLANN_LIBRARY_DIR ${openExternals_FLANN_LIBRARY_DIR})
	# set(WindMillCounter_FLANN_LIBRARY ${openExternals_FLANN_LIBRARY})
	
	# #pcl
	# set(WindMillCounter_PCL_DIR ${openExternals_PCL_DIR})
	# list(APPEND CMAKE_PREFIX_PATH ${WindMillCounter_PCL_DIR})
	
	# #ctk
	# set(WindMillCounter_CTK_DIR ${openExternals_CTK_DIR})
	# list( APPEND CMAKE_PREFIX_PATH ${WindMillCounter_CTK_DIR} )
else()
	message(FATAL_ERROR "Cannot not find openExternals project.")
endif()

# Eigen
include_directories( ${WindMillCounter_EIGEN_DIR} )

# OpenCV
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_package(OpenIGTLink REQUIRED)
include(${OpenIGTLink_USE_FILE})

########  WINDMILL COUNTER  ########
set(WindMillCounter_SRCS
	main.cxx
	WindMillCounter.cxx
	WMObject.cxx)
	
set(WindMillCounter_HDRS
	WindMillCounter.h
	WMObject.h)

add_executable(WindMillCounter ${WindMillCounter_SRCS} ${WindMillCounter_HDRS})
target_link_libraries(WindMillCounter ${OpenCV_LIBS} OpenIGTLink)

# set up the windows batch file for setting up the correct runtime paths.
# TODO: build a script for VS2010, VS2012 and VS2013.  Also flip between 32 and 64 bits when needed.
if(WIN32)
	if(MSVC11 AND CMAKE_CL_64 )
		set(BUILD_TYPE "Debug")
		configure_file(
			${CMAKE_SOURCE_DIR}/cmake/WindMillCounter-VS2012-64-dev.bat.in
			${CMAKE_BINARY_DIR}/../WindMillCounter-VS2012-64-devDebug.bat )
		set(BUILD_TYPE "Release")
		configure_file(
			${CMAKE_SOURCE_DIR}/cmake/WindMillCounter-VS2012-64-dev.bat.in
			${CMAKE_BINARY_DIR}/../WindMillCounter-VS2012-64-devRelease.bat )
	else()
		message(WARNING "No launch batch files created for this IDE environment.")
	endif()
elseif(APPLE)
    configure_file(
        ${CMAKE_SOURCE_DIR}/cmake/WindMillCounter-QtCreator-OSX-dev.sh.in
        ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/WindMillCounter-QtCreator-OSX-dev.command )
    file(COPY ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/WindMillCounter-QtCreator-OSX-dev.command DESTINATION ${CMAKE_BINARY_DIR}/../ FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
endif(WIN32)
