
IF(DYNAMIC_hogbox)
    ADD_DEFINITIONS(-DHOGBOXVIS_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DHOGBOX_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME hogboxVision)
SET(HEADER_PATH ${hogbox_SOURCE_DIR}/include/${LIB_NAME})


SET(TARGET_H
	${HEADER_PATH}/VideoStream.h
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
SET(TARGET_SRC
	HogVisionNode.cpp
	PlanarTrackedObject.cpp
	RTTPass.cpp
	TrackedObject.cpp
	VideoLayer.cpp
	VideoStream.cpp
	VisionRegistry.cpp
	CameraBasedTracker.cpp
	CameraCalibration.cpp
	HogTracker.cpp
)

SET(TARGET_LIBRARIES hogbox)

SET(TARGET_EXTERNAL_LIBRARIES     
    OSG_LIBRARY
    OSGDB_LIBRARY
    OSGUTIL_LIBRARY
    OSGVIEWER_LIBRARY
	OSGGA_LIBRARY
    OPENTHREADS_LIBRARY
)

SETUP_LIBRARY(${LIB_NAME})