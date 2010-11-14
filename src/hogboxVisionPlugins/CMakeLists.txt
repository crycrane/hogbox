#---------------------------------------------------
# OSG CMAKE SUPPORT
# (C) by Michael Wagner, mtw@shared-reality.com 2005
# (C) Eric Wing, Luigi Calori and Robert Osfield 2006-2007
#---------------------------------------------------

PROJECT(HOGBOXVIS_PLUGINS_MASTER)

IF(NOT DYNAMIC_HOGBOX)
    ADD_DEFINITIONS(-DHOGBOX_LIBRARY_STATIC)
ENDIF()

IF(NOT MSVC)
    SET(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}/${hogboxVis_PLUGINS}")
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${hogboxVis_PLUGINS}")
ENDIF()

SET(CMAKE_SHARED_MODULE_PREFIX ${hogboxVis_PLUGIN_PREFIX})

IF(MSVC80 OR MSVC90)
    IF(NOT HOGBOX_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
        SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    ENDIF()
ENDIF()

SET(TARGET_DEFAULT_PREFIX "hogboxvision_")
SET(TARGET_DEFAULT_LABEL_PREFIX "visionPlugin_")
SET(TARGET_COMMON_LIBRARIES
	hogbox
	hogboxDB
	hogboxVision
)

############################################################
#
#  Video/webcam plugins
#

IF(WIN32)
    ADD_SUBDIRECTORY(hogboxVisionDShowVideoPlugin)
    ADD_SUBDIRECTORY(hogboxVisionDShowWebCamPlugin)
ELSE(WIN32)
    IF(APPLE)
        #CMAKE_MINIMUM_REQUIRED(VERSION 2.6.0 FATAL_ERROR)
	IF(OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
            ADD_SUBDIRECTORY(hogboxVisionIPhoneWebCamPlugin)
	ENDIF()	
    ENDIF(APPLE)
ENDIF(WIN32)




##########to get all the variables of Cmake
#GET_CMAKE_PROPERTY(MYVARS VARIABLES)
#FOREACH(myvar ${MYVARS})
#    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/AllVariables.txt
#        "${myvar} -->${${myvar}}<-\n"
#    )
#ENDFOREACH(myvar)


