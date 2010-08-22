#---------------------------------------------------
# OSG CMAKE SUPPORT
# (C) by Michael Wagner, mtw@shared-reality.com 2005
# (C) Eric Wing, Luigi Calori and Robert Osfield 2006-2007
#---------------------------------------------------

PROJECT(HOGBOX_PLUGINS_MASTER)

IF(NOT DYNAMIC_HOGBOX)
    ADD_DEFINITIONS(-DHOGBOX_LIBRARY_STATIC)
ENDIF()

IF(NOT MSVC)
    SET(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}/${HOGBOX_PLUGINS}")
    SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${OSG_PLUGINS}")
ENDIF()

SET(CMAKE_SHARED_MODULE_PREFIX ${HOGBOX_PLUGIN_PREFIX})

IF(MSVC80 OR MSVC90)
    IF(NOT HOGBOX_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
        SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
    ENDIF()
ENDIF()

SET(TARGET_DEFAULT_PREFIX "hogboxxml_")
SET(TARGET_DEFAULT_LABEL_PREFIX "xmlPlugin_")
SET(TARGET_COMMON_LIBRARIES
	hogbox
	hogboxDB
)

############################################################
#
#  XmlLoader plugins
#
ADD_SUBDIRECTORY(hogboxHUDPlugin)
ADD_SUBDIRECTORY(hogboxPlugin)
ADD_SUBDIRECTORY(hogboxVisionPlugin)


##########to get all the variables of Cmake
#GET_CMAKE_PROPERTY(MYVARS VARIABLES)
#FOREACH(myvar ${MYVARS})
#    FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/AllVariables.txt
#        "${myvar} -->${${myvar}}<-\n"
#    )
#ENDFOREACH(myvar)


