# this does not try and find out which version of cudnn it finds. this may be a problem later on.

set(LIBRARY_PATHS
	${CUDA_SDK_ROOT_DIR}/lib/x64
	)
	
find_library(CUDNN_LIBRARY
	NAMES cudnn
	PATHS ${LIBRARY_PATHS}
	)
	
if(CUDNN_LIBRARY)
	set(CUDNN_FOUND TRUE)
	set(CUDNN_LIBRARIES ${CUDNN_LIBRARY} CACHE STRING "the cudnn lib")
endif()

mark_as_advanced(
	CUDNN_LIBRARIES
	)