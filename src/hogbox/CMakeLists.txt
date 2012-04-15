IF(DYNAMIC_hogbox)
    ADD_DEFINITIONS(-DHOGBOX_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DHOGBOX_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME hogbox)
SET(HEADER_PATH ${hogbox_SOURCE_DIR}/include/${LIB_NAME})

SET(LIB_PUBLIC_HEADERS
	${HEADER_PATH}/AnimateValue.h
	${HEADER_PATH}/AnimationPathControl.h
	${HEADER_PATH}/AnimationPathEventCallback.h
	${HEADER_PATH}/AnimationUtils.h
    ${HEADER_PATH}/AssetManager.h
    ${HEADER_PATH}/Callback.h
	${HEADER_PATH}/Export.h
	${HEADER_PATH}/Version.h
	${HEADER_PATH}/HogBoxBase.h
	${HEADER_PATH}/HogBoxHardwareRigTransform.h
	${HEADER_PATH}/HogBoxLight.h
	${HEADER_PATH}/HogBoxMaterial.h
	${HEADER_PATH}/HogBoxMesh.h
	${HEADER_PATH}/HogBoxNotifyHandler.h
	${HEADER_PATH}/HogBoxObject.h
	${HEADER_PATH}/HogBoxUtils.h
	${HEADER_PATH}/HogBoxViewer.h
	${HEADER_PATH}/Noise.h
	${HEADER_PATH}/SystemInfo.h
	${HEADER_PATH}/NPOTResizeCallback.h
    ${HEADER_PATH}/Quad.h
    ${HEADER_PATH}/TransformQuad.h
    ${hogbox_CONFIG_HEADER}
)

ADD_LIBRARY(${LIB_NAME}
    ${hogbox_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    AnimationPathController.cpp
    AnimationPathEventCallback.cpp
	AnimationUtils.cpp
    AssetManager.cpp
	HogBoxHardwareRigTransform.cpp
	HogBoxLight.cpp
	HogBoxMaterial.cpp
	HogBoxMesh.cpp
	HogBoxNotifyHandler.cpp
	HogBoxObject.cpp
	HogBoxUtils.cpp
	HogBoxViewer.cpp
	Noise.cpp
	SystemInfo.cpp
	NPOTResizeCallback.cpp
    Quad.cpp
    TransformQuad.cpp
	Version.cpp
    #${HOGBOX_VERSIONINFO_RC}
)

LINK_WITH_VARIABLES(${LIB_NAME}     
    OSG_LIBRARY
    OSGDB_LIBRARY
    OSGUTIL_LIBRARY
    OSGVIEWER_LIBRARY
	OSGGA_LIBRARY
	OSGANIMATION_LIBRARY
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
