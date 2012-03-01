
PROJECT(hogbox_PLUGINS_MASTER)


###########################################
# Settings for plugin
###########################################
IF(NOT MSVC)
    SET(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}/${hogbox_PLUGINS}")
ENDIF(NOT MSVC)

IF(MSVC80)
  IF(NOT hogbox_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
  ENDIF(NOT hogbox_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
ENDIF(MSVC80)

SET(CMAKE_SHARED_MODULE_PREFIX ${hogbox_PLUGIN_PREFIX})
SET(TARGET_DEFAULT_PREFIX "hogboxxml_")
SET(TARGET_DEFAULT_LABEL_PREFIX "xmlPlugin_")


SET(TARGET_SRC
    HogBoxXmlManager.cpp
)
SET(TARGET_H
	FeatureLevelXmlWrapper.h
	HogBoxLightXmlWrapper.h
	HogBoxMaterialXmlWrapper.h
	HogBoxObjectXmlWrapper.h
	HogBoxViewerXmlWrapper.h
	MeshMappingXmlWrapper.h
	OsgImageXmlWrapper.h
	OsgNodeXmlWrapper.h
	OsgShaderXmlWrapper.h
	OsgTextureXmlWrapper.h
    OsgTexture2DXmlWrapper.h
    OsgTextureRectangleXmlWrapper.h
    OsgTexture3DXmlWrapper.h
    OsgTextureCubeXmlWrapper.h
	OsgUniformXmlWrapper.h
)
#SET(TARGET_ADDED_LIBRARIES )

#### end var setup  ###
SETUP_XML_PLUGIN(hogbox)
