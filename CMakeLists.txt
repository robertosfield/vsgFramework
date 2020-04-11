cmake_minimum_required(VERSION 3.5)

project(vsgFramework)

# find the various depdendencies
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMakeModules/")
include(ExternalProject)

set(OUTPUT_BINDIR ${PROJECT_BINARY_DIR}/bin)
set(OUTPUT_LIBDIR ${PROJECT_BINARY_DIR}/lib)
set(OUTPUT_INCDIR ${PROJECT_BINARY_DIR}/include)
set(COMPONENTS_DIR ${PROJECT_BINARY_DIR}/components)
set(DOWNLOADS_DIR ${PROJECT_BINARY_DIR}/downloads)
set(PREFIX_PATH   ${CMAKE_PREFIX_PATH} ${PROJECT_BINARY_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
if(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})
else(WIN32)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_LIBDIR})
endif(WIN32)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BINDIR})

# set compilation to use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Change the default build type to Release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif(NOT CMAKE_BUILD_TYPE)


# Dynamic vs Static Linking
option(VSGFRAMEWORK_SHARED_LIBRARY "Set to ON to build libraries for dynamic linking.  Use OFF for static." ON)




macro(addExternal TARGET_NAME REPOSITORY TAG EXTRA_CMAKE_ARGS1 EXTRA_CMAKE_ARGS2 EXTRA_CMAKE_ARGS3 EXTRA_CMAKE_ARGS4)

    ExternalProject_Add(
        ${TARGET_NAME}

        PREFIX components

        GIT_REPOSITORY ${REPOSITORY}
        GIT_TAG ${TAG}

        UPDATE_COMMAND ""
        PATCH_COMMAND ""

        SOURCE_DIR "${COMPONENTS_DIR}/${TARGET_NAME}"
        CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR} -DCMAKE_PREFIX_PATH=${PREFIX_PATH} ${EXTRA_CMAKE_ARGS1} ${EXTRA_CMAKE_ARGS2} ${EXTRA_CMAKE_ARGS3} ${EXTRA_CMAKE_ARGS4}
    )
endmacro()

add_custom_target(clobber
    COMMAND git clean -d -f -x
)

find_package(Threads)

find_library(DL_LIBRARY dl)
if(NOT DL_LIBRARY)
    set(DL_LIBRARY "") # change from NOTFOUND to empty when passed to linker
endif()

if (BUILD_SHARED_LIBS)
    set(DEFAULT_LIB_SUFFIX ${CMAKE_SHARED_LIBRARY_SUFFIX})
else()
    set(DEFAULT_LIB_SUFFIX ${CMAKE_STATIC_LIBRARY_SUFFIX})
endif()

if (VULKAN_SDK)
    set(ENV{VULKAN_SDK} ${VULKAN_SDK})
endif()

find_package(Vulkan)
if (NOT VULKAN_FOUND)

    set(VulkanSDKVersion "1.2.131.2" CACHE STRING "Vulkan_SDK version to download")

    if (NOT EXISTS ${DOWNLOADS_DIR}/${VulkanSDKVersion})

        message("Will attempt to download : " ${VulkanSDKVersion})

        file(MAKE_DIRECTORY downloads)

        execute_process(COMMAND wget --no-cookies -O ${DOWNLOADS_DIR}/vulkansdk-linux-x86_64-${VulkanSDKVersion}.tar.gz https://sdk.lunarg.com/sdk/download/${VulkanSDKVersion}/linux/vulkansdk-linux-x86_64-${VulkanSDKVersion}.tar.gz)
        execute_process(COMMAND tar -zxf ${DOWNLOADS_DIR}/vulkansdk-linux-x86_64-${VulkanSDKVersion}.tar.gz -C ${DOWNLOADS_DIR})

    endif()

    if (EXISTS ${DOWNLOADS_DIR}/${VulkanSDKVersion})

        set(VULKAN_SDK ${DOWNLOADS_DIR}/${VulkanSDKVersion}/x86_64)
        set(ENV{VULKAN_SDK} ${VULKAN_SDK})
        set(EXTRA_CMAKE_ARGS_VULKAN_SDK -DVULKAN_SDK=${VULKAN_SDK})

        message("VULKAN_SDK = " $ENV{VULKAN_SDK})

        find_package(Vulkan)

    endif()

endif()


find_package(osg)
find_package(osgDB)
find_package(osgUtil)
if (NOT OSG_FOUND)

    message("In OSG Fallback setup")

    if (NOT EXISTS include)
        file(MAKE_DIRECTORY include)
    endif()

    set(TARGET_OSG OpenSceneGraph)

    if (WIN32)
        set(OSG_LIB_SUFFIX ".lib")
    else()
        set(OSG_LIB_SUFFIX ".so")
    endif()

    addExternal(${TARGET_OSG} "https://github.com/openscenegraph/OpenSceneGraph.git" "OpenSceneGraph-3.6" "-DLIB_POSTFIX=" "" "")
    set(OSG_INCLUDE_DIR ${OUTPUT_INCDIR} CACHE PATH "OSG header directory" FORCE)
    set(OSG_LIBRARIES ${OUTPUT_LIBDIR}/libosg${OSG_LIB_SUFFIX} CACHE FILEPATH "osg Library" FORCE)
    set(OSGDB_LIBRARIES ${OUTPUT_LIBDIR}/libosgDB${OSG_LIB_SUFFIX} CACHE FILEPATH "osgUtl Library" FORCE)
    set(OSGUTIL_LIBRARIES ${OUTPUT_LIBDIR}/libosgUtil${OSG_LIB_SUFFIX} CACHE FILEPATH "osgDB Library" FORCE)
    set(OSG_FOUND TRUE CACHE BOOL "OSG Library" FORCE)
endif()

find_package(vsg QUIET)
if (NOT vsg_FOUND)

    message("In VSG Fallback setup")

    if (NOT EXISTS include)
        file(MAKE_DIRECTORY include)
    endif()

    set(TARGET_VSG VulkanSceneGraph)
    addExternal(${TARGET_VSG} "https://github.com/vsg-dev/VulkanSceneGraph.git" "master" "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}" "${EXTRA_CMAKE_ARGS_VULKAN_SDK}" "" "")
    set(vsg_INCLUDE_DIR ${OUTPUT_INCDIR} CACHE PATH "vsg header directory" FORCE)
    set(vsg_INCLUDE_DIRS ${vsg_INCLUDE_DIR})
    set(vsg_LIBRARY ${OUTPUT_LIBDIR}/libvsg${DEFAULT_LIB_SUFFIX} CACHE FILEPATH "vsg Library" FORCE)
    set(vsg_LIBRARIES ${vsg_LIBRARY})
    set(vsg_FOUND TRUE CACHE BOOL "vsg Library" FORCE)
    if (${BUILD_SHARED_LIBS})
        set(vsg_DEFINITIONS vsg_SHARED_LIBRARY)
    endif()

    add_library(vsg::vsg UNKNOWN IMPORTED)

    set(WINDOWING_LIBS xcb)

    # note. need to make xcb entry in INTERFACE_LINK_LIBRARIES platform depedent.
    set_target_properties(vsg::vsg PROPERTIES
        IMPORTED_LOCATION "${vsg_LIBRARIES}"
        INTERFACE_LINK_LIBRARIES "Vulkan::Vulkan;Threads::Threads;${WINDOWING_LIBS}"
        INTERFACE_INCLUDE_DIRECTORIES "${vsg_INCLUDE_DIRS}")

endif()


find_package(osg2vsg QUIET)
if (NOT osg2vsg_FOUND)

    message("In OSG2VSG Fallback setup")

    find_package(glslang)

    if (NOT EXISTS include)
        file(MAKE_DIRECTORY include)
    endif()

    set(TARGET_OSG2VSG osg2vsg)

    if (vsg_DIR)
        set(EXTRA_ARGS1 "-Dvsg_DIR=${vsg_DIR}")
    endif()
    if (OSG_DIR)
        set(EXTRA_ARGS2 "-DOSG_DIR=${OSG_DIR}")
    endif()

    addExternal(${TARGET_OSG2VSG} "https://github.com/vsg-dev/osg2vsg.git" "master" "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}" "${EXTRA_ARGS1}" "${EXTRA_ARGS2}" "${EXTRA_CMAKE_ARGS_VULKAN_SDK}")
    if (TARGET_OSG OR TARGET_VSG)
        ExternalProject_Add_StepDependencies(${TARGET_OSG2VSG} configure ${TARGET_OSG} ${TARGET_VSG})
    endif()
    set(osg2vsg_INCLUDE_DIR ${OUTPUT_INCDIR} CACHE PATH "osg2vsg header directory" FORCE)
    set(osg2vsg_LIBRARY ${OUTPUT_LIBDIR}/libosg2vsg${DEFAULT_LIB_SUFFIX} CACHE FILEPATH "osg2vsg Library" FORCE)
    set(osg2vsg_LIBRARIES ${osg2vsg_LIBRARY})
    set(osg2vsg_FOUND TRUE CACHE BOOL "osg2vsg Library" FORCE)
    if (${BUILD_SHARED_LIBS})
        set(osg2vsg_DEFINITIONS osg2vsg_SHARED_LIBRARY)
    endif()

    add_library(osg2vsg::osg2vsg UNKNOWN IMPORTED)
    set_target_properties(osg2vsg::osg2vsg PROPERTIES
        IMPORTED_LOCATION "${osg2vsg_LIBRARIES}"
        INTERFACE_LINK_LIBRARIES "vsg::vsg;${OSG_LIBRARIES};${OSGUTIL_LIBRARIES};${OSGDB_LIBRARIES};${GLSLANG}"
        INTERFACE_INCLUDE_DIRECTORIES "${osg2vsg_INCLUDE_DIRS}")

endif()


find_package(vsgXchange QUIET)
if (NOT vsgXchange_FOUND)

    find_package(glslang)

    message("In VSGXCHANGE Fallback setup")

    if (NOT EXISTS include)
        file(MAKE_DIRECTORY include)
    endif()

    set(TARGET_VSGXCHANGE vsgXchange)

    if (vsg_DIR)
        set(EXTRA_ARGS1 "-Dvsg_DIR=${vsg_DIR}")
    endif()
    if (osg2vsg_DIR)
        set(EXTRA_ARGS2 "-Dosg2vsg_DIR=${Dosg2vsg_DIR}")
    endif()

    addExternal(${TARGET_VSGXCHANGE} "https://github.com/vsg-dev/vsgXchange.git" "master" "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}" "${EXTRA_ARGS1}" "${EXTRA_ARGS2}" "${EXTRA_CMAKE_ARGS_VULKAN_SDK}")
    if (TARGET_VSG OR TARGET_OSG2VSG)
        ExternalProject_Add_StepDependencies(${TARGET_VSGXCHANGE} configure ${TARGET_VSG} ${TARGET_OSG2VSG})
    endif()
    set(vsgXchange_INCLUDE_DIR ${OUTPUT_INCDIR} CACHE PATH "vsgXchange header directory" FORCE)
    set(vsgXchange_LIBRARY ${OUTPUT_LIBDIR}/libvsgXchange${DEFAULT_LIB_SUFFIX} CACHE FILEPATH "vsgXchange Library" FORCE)
    set(vsgXchange_LIBRARIES ${vsgXchange_LIBRARY})
    set(vsgXchange_FOUND TRUE CACHE BOOL "vsgXchange Library" FORCE)
    if (${BUILD_SHARED_LIBS})
        set(vsgXchange_DEFINITIONS vsgXchange_SHARED_LIBRARY)
    endif()

    add_library(vsgXchange::vsgXchange UNKNOWN IMPORTED)
    set_target_properties(vsgXchange::vsgXchange PROPERTIES
        IMPORTED_LOCATION "${vsgXchange_LIBRARIES}"
        INTERFACE_LINK_LIBRARIES "vsg::vsg;osg2vsg::osg2vsg;${GLSLANG}"
        INTERFACE_INCLUDE_DIRECTORIES "${vsgXchange_INCLUDE_DIRS}")

endif()

if (vsg_FOUND)
    add_subdirectory(applications)
endif()
