
IF(DYNAMIC_hogbox)
    ADD_DEFINITIONS(-DHOGBOXSTAGE_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DHOGBOX_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME hogboxStage)
SET(HEADER_PATH ${hogbox_SOURCE_DIR}/include/${LIB_NAME})

SET(LIB_PUBLIC_HEADERS
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
ADD_LIBRARY(${LIB_NAME}
    ${hogbox_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    Component.cpp
    EntityManager.cpp
    ComponentXmlManager.cpp
    RenderableComponent.cpp
)

LINK_INTERNAL(${LIB_NAME} hogbox hogboxDB hogboxHUD)

LINK_WITH_VARIABLES(${LIB_NAME}     
    OSG_LIBRARY
    OSGDB_LIBRARY
    OSGTEXT_LIBRARY
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