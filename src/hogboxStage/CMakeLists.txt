
IF(DYNAMIC_hogbox)
    ADD_DEFINITIONS(-DHOGBOXSTAGE_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DHOGBOX_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME hogboxStage)
SET(HEADER_PATH ${hogbox_SOURCE_DIR}/include/${LIB_NAME})

SET(TARGET_H
    ${HEADER_PATH}/Export.h
	${HEADER_PATH}/Entity.h
	${HEADER_PATH}/Component.h
	${HEADER_PATH}/ComponentEvent.h
	${HEADER_PATH}/ComponentEventCallback.h
	${HEADER_PATH}/EntityManager.h
	${HEADER_PATH}/EntityXmlWrapper.h
	${HEADER_PATH}/ComponentXmlWrapper.h
	${HEADER_PATH}/WorldTransformComponent.h
	${HEADER_PATH}/PhysicsComponent.h
	${HEADER_PATH}/CollidableComponent.h
	${HEADER_PATH}/RenderableComponent.h
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
    Component.cpp
    EntityManager.cpp
    ComponentXmlManager.cpp
    RenderableComponent.cpp
)

SET(TARGET_LIBRARIES hogbox hogboxDB hogboxHUD)

SET(TARGET_EXTERNAL_LIBRARIES       
    OSG_LIBRARY
    OSGDB_LIBRARY
    OSGTEXT_LIBRARY
    OSGUTIL_LIBRARY
    OSGVIEWER_LIBRARY
	OSGGA_LIBRARY
	OSGANIMATION_LIBRARY
    OPENTHREADS_LIBRARY
)

SETUP_LIBRARY(${LIB_NAME})