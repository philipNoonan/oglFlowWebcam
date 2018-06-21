SET(CAFFE_ROOT_DIR "$ENV{THIRD_PARTY_DIR}/caffe")

set(LIBRARY_PATHS
	~/usr/lib
	~/usr/local/lib
	/usr/lib
	/usr/local/lib
	${CAFFE_ROOT_DIR}/lib
	)

find_library(CAFFE_LIBRARY 
	NAMES caffe
	PATHS ${LIBRARY_PATHS}
	)
	
find_library(CAFFEPROTO_LIBRARY 
	NAMES caffeproto
	PATHS ${LIBRARY_PATHS}
	)
	
find_path(CAFFE_INCLUDE_DIR caffe/caffe.hpp
#    PATH_SUFFIXES libfreenect
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${CAFFE_ROOT_DIR}/include
	)
	

	
if(CAFFE_LIBRARY AND CAFFE_INCLUDE_DIR)
	set(CAFFE_FOUND TRUE)
	set(CAFFE_INCLUDE_DIRS ${CAFFE_INCLUDE_DIR} CACHE STRING "The include paths needed to use caffe")
    set(CAFFE_LIBRARIES ${CAFFE_LIBRARY} ${CAFFEPROTO_LIBRARY} CACHE STRING "The libraries needed to use caffe")
endif()

mark_as_advanced(
    CAFFE_INCLUDE_DIRS
    CAFFE_LIBRARIES
	)