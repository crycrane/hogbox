
IF(DYNAMIC_hogbox)
    ADD_DEFINITIONS(-DHOGBOXVIS_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DHOGBOX_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME hogboxVision)
SET(HEADER_PATH ${hogbox_SOURCE_DIR}/include/${LIB_NAME})


SET(LIB_PUBLIC_HEADERS
	${HEADER_PATH}/VideoStream.h
	${HEADER_PATH}/VideoTex2DCallBack.h
	${HEADER_PATH}/VisionRegistry.h
	${HEADER_PATH}/VisionRegistryWrappers.h
	${HEADER_PATH}/WebCamStream.h
	${HEADER_PATH}/CameraBasedTracker.h
	${HEADER_PATH}/CameraCalibration.h
	${HEADER_PATH}/Export.h
	${HEADER_PATH}/HogTracker.h
	${HEADER_PATH}/HogVisionNode.h
	${HEADER_PATH}/PlanarTrackedObject.h
	${HEADER_PATH}/RTTPass.h
	${HEADER_PATH}/TrackedObject.h
	${HEADER_PATH}/VideoFileStream.h
	${HEADER_PATH}/VideoLayer.h
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${hogbox_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
	HogVisionNode.cpp
	PlanarTrackedObject.cpp
	RTTPass.cpp
	TrackedObject.cpp
	VideoLayer.cpp
	VideoStream.cpp
	VideoTex2DCallBack.cpp
	VisionRegistry.cpp
	CameraBasedTracker.cpp
	CameraCalibration.cpp
	HogTracker.cpp
)

LINK_INTERNAL(${LIB_NAME} hogbox)

LINK_WITH_VARIABLES(${LIB_NAME}     
    OSG_LIBRARY
    OSGDB_LIBRARY
    OSGUTIL_LIBRARY
    OSGVIEWER_LIBRARY
	OSGGA_LIBRARY
    OPENTHREADS_LIBRARY
)
LINK_EXTERNAL(${LIB_NAME} ${OPENGL_LIBRARIES}) 
LINK_CORELIB_DEFAULT(${LIB_NAME})

SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES DEBUG_POSTFIX "d")
if(MSVC)
    SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES PREFIX "../")
    SET_TARGET_PROPERTIES(${LIB_NAME} PROPERTIES IMPORT_PREFIX "../")
endif(MSVC)

INCLUDE(ModuleInstall OPTIONAL)