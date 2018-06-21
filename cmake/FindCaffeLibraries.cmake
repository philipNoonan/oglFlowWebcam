SET(CAFFE_LIBRARIES_ROOT_DIR "$ENV{THIRD_PARTY_DIR}/caffe-libraries")

set(LIBRARY_PATHS
	~/usr/lib
	~/usr/local/lib
	/usr/lib
	/usr/local/lib
	${CAFFE_LIBRARIES_ROOT_DIR}/lib
	)

find_library(CAFFE_LIBRARIES_BOOST_FS_LIB 
	NAMES boost_filesystem-vc140-mt-1_61
	PATHS ${LIBRARY_PATHS}
	)
find_library(CAFFE_LIBRARIES_BOOST_SYS_LIB 
	NAMES boost_system-vc140-mt-1_61
	PATHS ${LIBRARY_PATHS}
	)	
find_library(CAFFE_LIBRARIES_HDF5_LIB 
	NAMES caffehdf5
	PATHS ${LIBRARY_PATHS}
	)
find_library(CAFFE_LIBRARIES_HDF5HL_LIB 
	NAMES caffehdf5_hl
	PATHS ${LIBRARY_PATHS}
	)
find_library(CAFFE_LIBRARIES_ZLIB_LIB 
	NAMES caffezlib
	PATHS ${LIBRARY_PATHS}
	)
find_library(CAFFE_LIBRARIES_GFLAGS_LIB 
	NAMES gflags
	PATHS ${LIBRARY_PATHS}
	)	
find_library(CAFFE_LIBRARIES_GLOG_LIB 
	NAMES glog
	PATHS ${LIBRARY_PATHS}
	)	
find_library(CAFFE_LIBRARIES_LEVELDB_LIB 
	NAMES leveldb
	PATHS ${LIBRARY_PATHS}
	)
find_file(CAFFE_LIBRARIES_LIBOPENBLAS_LIB 
	NAMES libopenblas.dll.a
	PATHS ${LIBRARY_PATHS}
	)
find_library(CAFFE_LIBRARIES_LIBPROTOBUF_LIB 
	NAMES libprotobuf
	PATHS ${LIBRARY_PATHS}
	)	
find_library(CAFFE_LIBRARIES_LMDB_LIB 
	NAMES lmdb
	PATHS ${LIBRARY_PATHS}
	)
find_library(CAFFE_LIBRARIES_SNAPPY_LIB 
	NAMES snappy
	PATHS ${LIBRARY_PATHS}
	)		

	
find_path(CAFFE_LIBRARIES_INCLUDE_DIR cblas.h
#    an example header file that should exist here
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${CAFFE_LIBRARIES_ROOT_DIR}/include
	)
	
find_path(CAFFE_BOOST_INCLUDE_DIR boost/align.hpp
#    an example header file that should exist here
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${CAFFE_LIBRARIES_ROOT_DIR}/include/boost-1_61
	)
	

	
if(CAFFE_LIBRARY AND CAFFE_INCLUDE_DIR)
	set(CAFFE_FOUND TRUE)
	set(CAFFE_LIBRARIES_INCLUDE_DIRS ${CAFFE_LIBRARIES_INCLUDE_DIR} ${CAFFE_BOOST_INCLUDE_DIR} CACHE STRING "The include paths needed to use caffe libraries")
    set(CAFFE_LIBRARIES_LIBRARIES ${CAFFE_LIBRARIES_BOOST_FS_LIB} ${CAFFE_LIBRARIES_BOOST_SYS_LIB} ${CAFFE_LIBRARIES_HDF5_LIB} ${CAFFE_LIBRARIES_HDF5HL_LIB} ${CAFFE_LIBRARIES_ZLIB_LIB} ${CAFFE_LIBRARIES_GFLAGS_LIB} ${CAFFE_LIBRARIES_GLOG_LIB} ${CAFFE_LIBRARIES_LEVELDB_LIB} ${CAFFE_LIBRARIES_LIBOPENBLAS_LIB} ${CAFFE_LIBRARIES_LIBPROTOBUF_LIB} ${CAFFE_LIBRARIES_LMDB_LIB} ${CAFFE_LIBRARIES_SNAPPY_LIB} CACHE STRING "The libraries needed to use caffe libraries")
endif()

mark_as_advanced(
    CAFFE_LIBRARIES_INCLUDE_DIRS
    CAFFE_LIBRARIES_LIBRARIES
	)