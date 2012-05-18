CMAKE_MINIMUM_REQUIRED(VERSION 2.4.5 FATAL_ERROR)

if(COMMAND cmake_policy)
    # Works around warnings libraries linked against that don't
    # have absolute paths (e.g. -lpthreads)
    cmake_policy(SET CMP0003 NEW)

    # Works around warnings about escaped quotes in ADD_DEFINITIONS
    # statements.
    cmake_policy(SET CMP0005 NEW)

    # cmake-2.6.1 introduces policy cmp0008 decide how to treat full path libraries that do not appear to be valid library file names
    # quote from cvslog "Such libraries worked by accident in the VS IDE and Xcode generators in CMake 2.4 and below."
    if(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} GREATER 4 AND ${CMAKE_PATCH_VERSION} GREATER 0)
        cmake_policy(SET CMP0008 OLD)
    endif()
endif()

#######################################
# Versions
#######################################
SET(hogbox_MAJOR_VERSION 0)
SET(hogbox_MINOR_VERSION 2)
SET(hogbox_PATCH_VERSION 0)
SET(hogbox_SOVERSION 1)

SET(hogbox_RELEASE_CANDIDATE 0)

SET(hogbox_VERSION ${hogbox_MAJOR_VERSION}.${hogbox_MINOR_VERSION}.${hogbox_PATCH_VERSION})
SET(hogbox_SONAMES TRUE)

SET(hogbox_PLUGINS hogboxPlugins-${hogbox_VERSION})
SET(hogbox_PLUGINS hogboxPlugins-${hogbox_VERSION})

SET(hogbox_PLUGIN_PREFIX "")
IF (CYGWIN)
    SET(hogbox_PLUGIN_PREFIX "cygwin_")
ENDIF()
IF(MINGW)
    SET(hogbox_PLUGIN_PREFIX "mingw_")
ENDIF()

SET(hogboxVis_PLUGINS hogboxVisPlugins-${hogbox_VERSION})

SET(hogboxVis_PLUGIN_PREFIX "")
IF (CYGWIN)
    SET(hogboxVis_PLUGIN_PREFIX "cygwin_")
ENDIF()
IF(MINGW)
    SET(hogboxVis_PLUGIN_PREFIX "mingw_")
ENDIF()

PROJECT(hogbox)

#
SET(CMAKE_MODULE_PATH "${hogbox_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
SET(SOURCE_DIR ${hogbox_SOURCE_DIR})
SET(OSG_DIR "${CMAKE_INSTALL_PREFIX}" CACHE STRING "Path where to find the osg installation")


# IPHONE_PORT@tom
# Trying to get CMake to generate an XCode IPhone project, current efforts are to get iphoneos sdk 3.1 working
# Added option which needs manually setting to select the IPhone SDK for building. We can only have one of the below 
# set to true. Should realy have an OSG_BUILD_PLATFORM variable that we set to our desired platform
OPTION(OSG_BUILD_PLATFORM_IPHONE "Enable IPhoneSDK Device support" OFF)
OPTION(OSG_BUILD_PLATFORM_IPHONE_SIMULATOR "Enable IPhoneSDK Simulator support" OFF)
IF(OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
  
  #you need to manually set the default sdk version here
  SET (IPHONE_SDKVER "4.1")
  #the below is taken from ogre, it states the gcc stuff needs to happen before PROJECT() is called. I've no clue if we even need it
  # Force gcc <= 4.2 on iPhone
  include(CMakeForceCompiler)
  CMAKE_FORCE_C_COMPILER(gcc-4.2 GNU)
  CMAKE_FORCE_CXX_COMPILER(gcc-4.2 GNU)
  SET(GCC_THUMB_SUPPORT NO)

  #set either the device sdk or the simulator sdk. Can't find away to separate these in the same project
  IF(OSG_BUILD_PLATFORM_IPHONE)
    SET (IPHONE_DEVROOT "/Developer/Platforms/iPhoneOS.platform/Developer")
    SET (IPHONE_SDKROOT "${IPHONE_DEVROOT}/SDKs/iPhoneOS${IPHONE_SDKVER}.sdk")
  ELSE()
    SET (IPHONE_DEVROOT "/Developer/Platforms/iPhoneSimulator.platform/Developer")
    SET (IPHONE_SDKROOT "${IPHONE_DEVROOT}/SDKs/iPhoneSimulator${IPHONE_SDKVER}.sdk")
  ENDIF()
ENDIF ()
# IPHONE_PORT@tom


#
## Option to enable Android build using AndroidNDK
OPTION(OSG_BUILD_PLATFORM_ANDROID OFF)
IF(OSG_BUILD_PLATFORM_ANDROID)

    CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

    INCLUDE(OsgAndroidMacroUtils)
    SET(ANDROID TRUE)
    IF(NOT ANDROID_PLATFORM)
        MESSAGE("Warning Android Platform version NOT defined, Default selected version: 5")
        SET(ANDROID_PLATFORM 5)
    ENDIF()
    IF(NOT ANDROID_ABI)
        MESSAGE("Warning Android ABI version NOT defined, Default selection: armeabi armeabi-v7a")
        SET(ANDROID_ABI "armeabi armeabi-v7a")
    ENDIF()
    IF(NOT ANDROID_STL)
        MESSAGE("Warning Android STL NOT defined, Default selection: gnustl_static")
        SET(ANDROID_STL "gnustl_static")
    ENDIF()
    IF(ANDROID_DEBUG)
        MESSAGE("Warning Android Build is in debug mode")
        SET(ANDROID_RELEASE_OPTIM "debug")
    ELSE()
        MESSAGE("Warning Android Build is in release mode")
        SET(ANDROID_RELEASE_OPTIM "release")
    ENDIF()
    IF(ANDROID_NEON)
        MESSAGE("Warning Android NEON optimizations enabled, this will not be available on all armeabi-v7a devices ie. Tegra2")
        SET(ANDROID_OPTIM_NEON true)
    ELSE()
        SET(ANDROID_OPTIM_NEON false)
    ENDIF()
    IF(ANDROID_ARM32)
        MESSAGE("Warning ARM 32bit instruction set will be used")
        SET(ANDROID_OPTIM_ARM32 true)
    ELSE()
        SET(ANDROID_OPTIM_ARM32 false)
    ENDIF()

    FIND_PACKAGE(AndroidNDK REQUIRED)

    SET(OSG_ANDROID_TEMPLATES "${CMAKE_SOURCE_DIR}/PlatformSpecifics/Android")

    SET(J  "4" CACHE STRING "how many processes for make -j <J>")

    ADD_CUSTOM_COMMAND(
        OUTPUT    Android-OpenSceneGraph
        DEPENDS   ${CMAKE_BINARY_DIR}/Android.mk
        COMMAND   "${ANDROID_NDK}/ndk-build"
        ARGS      --directory=${CMAKE_BINARY_DIR} NDK_APPLICATION_MK=Application.mk -j${J} NDK_LOG=1
    )

    ADD_CUSTOM_TARGET(ndk ALL echo
          DEPENDS Android-Hogbox
    )
    install(DIRECTORY include/ DESTINATION include/
        PATTERN ".svn" EXCLUDE
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/include/ DESTINATION include/
    )
    install(DIRECTORY ${CMAKE_BINARY_DIR}/obj/ DESTINATION obj/
        PATTERN ".svn" EXCLUDE
        PATTERN "objs" EXCLUDE
    )

    ADD_DEFINITIONS(-DANDROID)

ENDIF()


#######################################
# Dynamic vs Static Linking
#######################################
OPTION(DYNAMIC_hogbox "Set to ON to build hogbox for dynamic linking.  Use OFF for static." ON)
IF (DYNAMIC_hogbox)
    SET(hogbox_USER_DEFINED_DYNAMIC_OR_STATIC "SHARED")
ELSE (DYNAMIC_hogbox)
    SET(hogbox_USER_DEFINED_DYNAMIC_OR_STATIC "STATIC")
    SET(CMAKE_MODULE_CXX_FLAGS_DEBUG "-pthread")
ENDIF(DYNAMIC_hogbox)


#######################################
# Library files
#######################################
FIND_PACKAGE( OpenGL REQUIRED )
FIND_PACKAGE( OSG REQUIRED )


# Make the headers visible to everything
IF(NOT ${PROJECT_BINARY_DIR} EQUAL ${PROJECT_SOURCE_DIR})
   INCLUDE_DIRECTORIES(BEFORE ${PROJECT_BINARY_DIR}/include)
ENDIF(NOT ${PROJECT_BINARY_DIR} EQUAL ${PROJECT_SOURCE_DIR})


INCLUDE_DIRECTORIES(BEFORE
    ${OPENGL_INCLUDE_DIR}
    ${OSG_INCLUDE_DIR}
    ${SOURCE_DIR}/include
)


SET(GL_LIBS
  ${OPENGL_gl_LIBRARY}
  ${OPENGL_glu_LIBRARY}
)

# MESSAGE( "project path: ${PROJECT_BINARY_DIR}" )
# MESSAGE( "source  path: ${hogbox_SOURCE_DIR}" )


########################################################################################################
##### these were settings located in SetupCommon.cmake used in Luigi builds.... find out what are useful
########################################################################################################
#luigi#SET(CMAKE_VERBOSE_MAKEFILE TRUE)
#luigi#SET(CMAKE_SKIP_RPATH TRUE)
#luigi#SET(CMAKE_SKIP_RULE_DEPENDENCY TRUE)
#luigi#IF(UNIX)
#luigi#    LIST_CONTAINS(contains "g++" ${CMAKE_CXX_COMPILER_LIST})
#luigi#    IF (contains)
#luigi#        MESSAGE(${MY_MESSAGE_DEFAULT} "${CMAKE_CURRENT_LIST_FILE}:${CMAKE_CURRENT_LIST_LINE} setting  CMAKE_CXX_COMPILER to g++")
#luigi#        SET(CMAKE_CXX_COMPILER "g++")
#luigi#        SET(CMAKE_CXX_COMPILER_LOADED 2)
#luigi#        SET(CMAKE_CXX_COMPILER_WORKS 2)
#luigi#    ENDIF (contains)
#luigi#    SET(CMAKE_CXX_FLAGS_RELEASE "-O2")
#luigi#    SET(CMAKE_CXX_FLAGS_DEBUG "-ggdb -gstabs")
#luigi#ENDIF(UNIX)
########################################################################################################

#######################################
# Solution definition
#######################################
IF(WIN32)

    IF(MSVC)
        ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)

        # This option is to enable the /MP switch for Visual Studio 2005 and above compilers
        OPTION(WIN32_USE_MP "Set to ON to build hogbox with the /MP option (Visual Studio 2005 and above)." OFF)
        MARK_AS_ADVANCED(WIN32_USE_MP)
        IF(WIN32_USE_MP)
            SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
        ENDIF(WIN32_USE_MP)

    ENDIF(MSVC)

    # Both Cygwin and Msys need -DNOMINMAX ???
    IF(UNIX)
        ADD_DEFINITIONS(-DNOMINMAX)
    ENDIF(UNIX)

########################################################################################################
# the following options are MSVC specific,
# the first OSG_MSVC_VERSIONED_DLL activate a custom build-time layout that should allow to run examples and application
# fron bin folder without requiring installation step.
# it also prepend "osg${OPENSCENEGRAPH_SOVERSION}-" to only .dll files, leaving .lib files untouched in lib
# it also use a hack to get rid of Debug and Release folder in MSVC projects
# all the .dll and .pdb are in bin and all the .lib and .exp are in lib
#
# the second option disable incremental linking in debug build , that is enabled by default by CMake
##########################################################################################################

    IF(MSVC)
        IF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            MESSAGE("Warning:  disabling versioned options 2.4.6 exibits inconsintencies in .pdb naming, at least under MSVC, suggested upgrading at least to 2.4.7")
            SET(hogbox_MSVC_VERSIONED_DLL OFF)
            SET(hogbox_MSVC_DEBUG_INCREMENTAL_LINK ON)
        ELSE(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
            OPTION(hogbox_MSVC_VERSIONED_DLL "Set to ON to build OpenSceneGraph with versioned dll names" ON)
            MARK_AS_ADVANCED(hogbox_MSVC_VERSIONED_DLL)
            OPTION(hogbox_MSVC_DEBUG_INCREMENTAL_LINK "Set to OFF to build without incremental linking in debug (release is off by default)" OFF)
            MARK_AS_ADVANCED(hogbox_MSVC_DEBUG_INCREMENTAL_LINK)
            IF(NOT hogbox_MSVC_DEBUG_INCREMENTAL_LINK)
                SET(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
                SET(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
                SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL:NO")
            ENDIF(NOT hogbox_MSVC_DEBUG_INCREMENTAL_LINK)
        ENDIF(${CMAKE_MAJOR_VERSION} EQUAL 2 AND ${CMAKE_MINOR_VERSION} EQUAL 4 AND ${CMAKE_PATCH_VERSION} LESS 7)
    ENDIF(MSVC)

ENDIF(WIN32)


# Here we apparantly do some funky stuff with making the bin/ and lib/
# folders which is probably needed to work around a very old CMake bug?

#SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin/${CMAKE_SYSTE_NAME})
SET(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
MAKE_DIRECTORY(${OUTPUT_BINDIR})
IF(MSVC AND NOT MSVC_IDE)
    MAKE_DIRECTORY(${OUTPUT_BINDIR}/${hogbox_PLUGINS})
    MAKE_DIRECTORY(${OUTPUT_BINDIR}/${hogboxVis_PLUGINS})
ENDIF(MSVC AND NOT MSVC_IDE)

#SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib/${CMAKE_SYSTE_NAME})
SET(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
MAKE_DIRECTORY(${OUTPUT_LIBDIR})
IF(NOT MSVC OR MSVC_IDE)
    MAKE_DIRECTORY(${OUTPUT_LIBDIR}/${hogbox_PLUGINS})
    MAKE_DIRECTORY(${OUTPUT_LIBDIR}/${hogboxVis_PLUGINS})
ENDIF(NOT MSVC OR MSVC_IDE)

# On CMake 2.4.x use EXECUTABLE_OUTPUT_PAhTH and LIBRARY_OUTPUT_PATH and later
# we work around the DLL placement by use of the PREFIX target property hack
#
# On CMake 2.6.x use the newly minted CMAKE_LIBRARY_OUTPUT_DIRECTORY,
# CMAKE_ARCHIVE_OUTPUT_DIRECTORY & CMAKE_RUNTIME_OUTPUT_DIRECTORY

IF(CMAKE_MAJOR_VERSION EQUAL 2 AND CMAKE_MINOR_VERSION LESS 5)
    SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_BINDIR})
    SET(LIBRARY_OUTPUT_PATH    ${OUTPUT_LIBDIR})
ELSE()
    # If CMake >= 2.6.0
    SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    IF(WIN32)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
    ELSE(WIN32)
        SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
    ENDIF(WIN32)
ENDIF()



#Solution
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
SET(EXAMPLE_PREFIX hogbox_)
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})


INCLUDE(hogboxMacroUtils)


IF(CMAKE_COMPILER_IS_GNUCC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCC)
IF(CMAKE_COMPILER_IS_GNUCXX)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmessage-length=0")
ENDIF(CMAKE_COMPILER_IS_GNUCXX)



################################################################################
# maintainer options
################################################################################
OPTION(hogbox_MAINTAINER "Enable hogbox maintainer build methods, such as making svn branches, tags, updating ChangeLog." OFF)
IF (hogbox_MAINTAINER)

    SET(hogbox_SVN "trunk")
    #SET(OPENSCENEGRAPH_SVN "branches")
    SET(hogbox_BRANCH HogBox-${hogbox_MAJOR_VERSION}.${hogbox_MINOR_VERSION})

    #
    # Provide target for tagging a release
    #
    SET(SVNCOMMAND svn)
    SET(SVNTRUNKDIR     https://hogbox.googlecode.com/svn/trunk)
    SET(SVNTAGDIR       https://hogbox.googlecode.com/svn/tags)
    SET(SVNBRANCHDIR    https://hogbox.googlecode.com/svn/branch)

    IF   (hogbox_SVN STREQUAL "trunk")
        SET(SVNSOURCEDIR ${SVNTRUNKDIR})
    ELSE (Ohogbox_SVN STREQUAL "trunk")
        SET(SVNSOURCEDIR ${SVNBRANCH_DIR}/${hogbox_BRANCH})
    ENDIF(hogbox_SVN STREQUAL "trunk")


    IF   (hogbox_RELEASE_CANDIDATE EQUAL 0)
        SET(RELEASE_NAME hogbox-${hogbox_VERSION})
    ELSE (hogbox_RELEASE_CANDIDATE EQUAL 0)
        SET(RELEASE_NAME hogbox-${hogbox_VERSION}-rc${hogbox_RELEASE_CANDIDATE})
    ENDIF(hogbox_RELEASE_CANDIDATE EQUAL 0)


    ADD_CUSTOM_TARGET(tag-test
        COMMAND echo ${SVNCOMMAND} copy ${SVNSOURCEDIR} ${SVNTAGDIR}/${RELEASE_NAME} -m "Release ${RELEASE_NAME}"
    )

    ADD_CUSTOM_TARGET(tag-run
        COMMAND ${SVNCOMMAND} copy ${SVNSOURCEDIR} ${SVNTAGDIR}/${RELEASE_NAME} -m "Release ${RELEASE_NAME}"
    )

    ADD_CUSTOM_TARGET(branch-test
        COMMAND echo ${SVNCOMMAND} copy ${SVNSOURCEDIR} ${SVNBRANCHDIR}/${hogbox_BRANCH} -m "Branch ${hogbox_BRANCH}"
    )

    ADD_CUSTOM_TARGET(branch-run
        COMMAND ${SVNCOMMAND} copy ${SVNSOURCEDIR} ${SVNBRANCHDIR}/${hogbox_BRANCH} -m "Branch ${hogbox_BRANCH}"
    )

    #
    # Provide target for generating ChangeLog
    #
    SET(GENERATELOGS svn2cl)

    ADD_CUSTOM_TARGET(ChangeLog
        COMMAND ${SVNCOMMAND} update
        #COMMAND ${SVNCOMMAND} up
        #COMMAND ${GENERATELOGS}
        COMMAND ${SVNCOMMAND} log --xml --verbose | xsltproc doc/scripts/svn2cl.xsl - > ChangeLog
    )
    
ENDIF(hogbox_MAINTAINER)



################################################################################
# present the packaging option only if we have the cpack command defined (effectively >= 2.6.0)
################################################################################
IF(CMAKE_CPACK_COMMAND)
    OPTION(BUILD_hogbox_PACKAGES "Set to ON to generate CPack configuration files and packaging targets" OFF)
    IF(BUILD_hogbox_PACKAGES)
      INCLUDE(hogboxCPack)
    ENDIF(BUILD_hogbox_PACKAGES)
ENDIF(CMAKE_CPACK_COMMAND)


################################################################################
# Create bin and lib directories if required
################################################################################
#IF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
#   FILE(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/lib/${OSG_PLUGINS})
#ENDIF("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")


################################################################################
# Compile subdirectory
################################################################################
IF(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

#Main libs, plugins etc
ADD_SUBDIRECTORY(src)

#Example apps
ADD_SUBDIRECTORY(examples)


# IPHONE_PORT@tom
#Here we check if the user specified IPhone SDK
IF(OSG_BUILD_PLATFORM_IPHONE OR OSG_BUILD_PLATFORM_IPHONE_SIMULATOR)
	
	#set iphone arch and flags taken from http://sites.google.com/site/michaelsafyan/coding/resources/how-to-guides/cross-compile-for-the-iphone/how-to-cross-compile-for-the-iphone-using-cmake
	IF(OSG_BUILD_PLATFORM_IPHONE)
		SET(CMAKE_OSX_ARCHITECTURES "armv6;armv7" CACHE STRING "Build architectures for iOS" FORCE)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -miphoneos-version-min=3.1 -mno-thumb -arch armv6 -pipe -no-cpp-precomp" CACHE STRING "Flags used by the compiler during all build types." FORCE)
	ELSE()
		#simulator uses i386 architectures
		SET(CMAKE_OSX_ARCHITECTURES "i386;x86_64" CACHE STRING "Build architectures for iOS Simulator" FORCE)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -miphoneos-version-min=3.1 -mno-thumb -arch i386 -pipe -no-cpp-precomp" CACHE STRING "Flags used by the compiler during all build types." FORCE)
	ENDIF()

	#here we set the specific iphone sdk version. We can only set either device or simulator sdk. So if you want both you currently have to have two seperate projects
	SET(CMAKE_OSX_SYSROOT "${IPHONE_SDKROOT}" CACHE STRING "System root for iOS" FORCE)

	#hack, force link to opengles
	set(CMAKE_EXE_LINKER_FLAGS "-framework Foundation -framework OpenGLES")
			
	#use the IPhone windowing system
	SET(OSG_WINDOWING_SYSTEM "IPhone" CACHE STRING "Forced IPhone windowing system on iOS"  FORCE)
	SET(OSG_DEFAULT_IMAGE_PLUGIN_FOR_OSX "imageio " CACHE STRING "Forced imageio default image plugin for iOS" FORCE)

	#I think this or similar will be required for IPhone apps
	OPTION(OSG_BUILD_APPLICATION_BUNDLES "Enable the building of applications and examples as OSX Bundles" ON)
ENDIF()


################################################################################
# Set Config file
################################################################################
SET(hogbox_CONFIG_HEADER "${PROJECT_BINARY_DIR}/include/hogbox/Config.h")
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/src/hogbox/Config.in"
               "${hogbox_CONFIG_HEADER}")
# INSTALL_FILES(/include/osg/ FILES "${OPENSCENEGRAPH_CONFIG_HEADER}")


################################################################################
### uninstall target
################################################################################
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)
ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

#
IF(ANDROID)
    message(STATUS "Creating Android Makefile Master files" )
    configure_file("${OSG_ANDROID_TEMPLATES}/Android.mk.master.in" "${CMAKE_BINARY_DIR}/Android.mk")
    configure_file("${OSG_ANDROID_TEMPLATES}/Application.mk.master.in" "${CMAKE_BINARY_DIR}/Application.mk")
    configure_file("${OSG_ANDROID_TEMPLATES}/AndroidManifest.xml.master.in" "${CMAKE_BINARY_DIR}/AndroidManifest.xml")
ENDIF(ANDROID)