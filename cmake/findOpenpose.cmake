SET(OPENPOSE_ROOT_DIR "$ENV{THIRD_PARTY_DIR}/openpose")

set(LIBRARY_PATHS
	~/usr/lib
	~/usr/local/lib
	/usr/lib
	/usr/local/lib
	${OPENPOSE_ROOT_DIR}/lib
	)

find_library(OPENPOSE_LIBRARY 
	NAMES OpenPose
	PATHS ${LIBRARY_PATHS}
	)
	
find_path(OPENPOSE_INCLUDE_DIR openpose/headers.hpp
 	~/usr/include
	~/usr/local/include
	/usr/include
	/usr/local/include
	${OPENPOSE_ROOT_DIR}/include
	)
	
	
if(OPENPOSE_LIBRARY AND OPENPOSE_INCLUDE_DIR)
	set(OPENPOSE_FOUND TRUE)
	set(OPENPOSE_INCLUDE_DIRS ${OPENPOSE_INCLUDE_DIR} CACHE STRING "The include paths needed to use openpose")
    set(OPENPOSE_LIBRARIES ${OPENPOSE_LIBRARY} CACHE STRING "The libraries needed to use openpose")
endif()

mark_as_advanced(
    OPENPOSE_INCLUDE_DIRS
    OPENPOSE_LIBRARIES
	)