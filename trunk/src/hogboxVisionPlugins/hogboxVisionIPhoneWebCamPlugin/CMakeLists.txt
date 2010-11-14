
PROJECT(hogboxVis_PLUGINS_MASTER)

FIND_PACKAGE( DirectShow REQUIRED )

INCLUDE_DIRECTORIES( ${DIRECTSHOW_INLUDE_DIRS} )
SET (TARGET_EXTERNAL_LIBRARIES AVFoundation CoreVideo CoreMedia)

#######################################
# Plugin prefix
#######################################
SET(hogboxVis_PLUGIN_PREFIX "")
IF (CYGWIN)
    SET(hogboxVis_PLUGIN_PREFIX "cygwin_")
ENDIF(CYGWIN)
IF(MINGW)
    SET(hogboxVis_PLUGIN_PREFIX "mingw_")
ENDIF(MINGW)


###########################################
# Settings for plugin
###########################################
IF(NOT MSVC)
    SET(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}/${hogboxVis_PLUGINS}")
ENDIF(NOT MSVC)

IF(MSVC80)
  IF(NOT hogboxVis_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
  ENDIF(NOT hogboxVis_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
ENDIF(MSVC80)

SET(CMAKE_SHARED_MODULE_PREFIX ${hogboxVis_PLUGIN_PREFIX})
SET(TARGET_DEFAULT_PREFIX "hogboxvision_webcam_")
SET(TARGET_DEFAULT_LABEL_PREFIX "visionPlugin_webcam_")


SET(TARGET_SRC
    IPhoneCameraController.mm
    IPhoneWebCamStream.mm
)
SET(TARGET_H
    IPhoneCameraController.h
    IPhoneWebCamStream.h
)


SET(TARGET_ADDED_LIBRARIES hogboxVision)
##SET(TARGET_LIBRARIES_VARS DIRECTSHOW_LIBRARY )

#### end var setup  ###
SETUP_VISION_PLUGIN(iphone iphone)
SET_TARGET_PROPERTIES("${TARGET_DEFAULT_PREFIX}${TARGET_NAME}" PROPERTIES LINK_FLAGS "/NODEFAULTLIB:atlthunk")
