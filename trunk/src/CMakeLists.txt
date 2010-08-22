#the old construct SUBDIRS( was substituded by ADD_SUBDIRECTORY that is to be preferred according on CMake docs.
FOREACH( mylibfolder 
        hogbox
        hogboxDB 
        hogboxHUD 
        hogboxVision 
        hogboxXmlPlugins
		hogboxVisionPlugins
    )

    ADD_SUBDIRECTORY(${mylibfolder})

ENDFOREACH()

#does nothing as we don't have the osg macros
IF(MSVC80 OR MSVC90)
  OPTION(hogbox_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS "Generate or not manifests files under VS8 for dynamically loaded dlls" ON)
ENDIF()

