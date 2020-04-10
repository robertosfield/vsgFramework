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
    message("In OSG Fallback setup")
endif()

find_package(vsg)
if (NOT vsg_FOUND)
    set(TARGET_VSG VulkanSceneGraph)
    addExternal(${TARGET_VSG} "https://github.com/vsg-dev/VulkanSceneGraph.git" "master" "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}" "${EXTRA_CMAKE_ARGS_VULKAN_SDK}" "" "")
    set(VSG_INCLUDE_DIR ${OUTPUT_INCDIR} CACHE PATH "vsg header directory" FORCE)
    set(VSG_LIBRARY ${OUTPUT_LIBDIR}/libvsg${DEFAULT_LIB_SUFFIX} CACHE FILEPATH "vsg Library" FORCE)
    set(VSG_LIBRARIES ${VSG_LIBRARY})
    set(VSG_FOUND TRUE CACHE BOOL "vsg Library" FORCE)
    if (${BUILD_SHARED_LIBS})
        set(VSG_DEFINITIONS VSG_SHARED_LIBRARY)
    endif()
    message("In VSG Fallback setup")
endif()


find_package(osg2vsg)
if (NOT osg2vsg_FOUND)
    set(TARGET_OSG2VSG osg2vsg)

    if (VSG_DIR)
        set(EXTRA_ARGS1 "-DVSG_DIR=${VSG_DIR}")
    endif()
    if (OSG_DIR)
        set(EXTRA_ARGS2 "-DOSG_DIR=${OSG_DIR}")
    endif()

    addExternal(${TARGET_OSG2VSG} "https://github.com/vsg-dev/osg2vsg.git" "master" "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}" "${EXTRA_ARGS1}" "${EXTRA_ARGS2}" "${EXTRA_CMAKE_ARGS_VULKAN_SDK}")
    if (TARGET_OSG OR TARGET_VSG)
        ExternalProject_Add_StepDependencies(${TARGET_OSG2VSG} configure ${TARGET_OSG} ${TARGET_VSG})
    endif()
    set(OSG2VSG_INCLUDE_DIR ${OUTPUT_INCDIR} CACHE PATH "osg2vsg header directory" FORCE)
    set(OSG2VSG_LIBRARY ${OUTPUT_LIBDIR}/libosg2vsg${DEFAULT_LIB_SUFFIX} CACHE FILEPATH "osg2vsg Library" FORCE)
    set(OSG2VSG_LIBRARIES ${OSG2VSG_LIBRARY})
    set(OSG2VSG_FOUND TRUE CACHE BOOL "osg2vsg Library" FORCE)
    if (${BUILD_SHARED_LIBS})
        set(OSG2VSG_DEFINITIONS OSG2VSG_SHARED_LIBRARY)
    endif()
    message("In OSG2VSG Fallback setup")
endif()


find_package(vsgXchange)
if (NOT vsgXchange_FOUND)
    set(TARGET_VSGXCHANGE vsgXchange)

    if (VSG_DIR)
        set(EXTRA_ARGS1 "-DVSG_DIR=${VSG_DIR}")
    endif()
    if (OSG2VSG_DIR)
        set(EXTRA_ARGS2 "-DOSG2VSG_DIR=${DOSG2VSG_DIR}")
    endif()

    addExternal(${TARGET_VSGXCHANGE} "https://github.com/vsg-dev/vsgXchange.git" "master" "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}" "${EXTRA_ARGS1}" "${EXTRA_ARGS2}" "${EXTRA_CMAKE_ARGS_VULKAN_SDK}")
    if (TARGET_VSG OR TARGET_OSG2VSG)
        ExternalProject_Add_StepDependencies(${TARGET_VSGXCHANGE} configure ${TARGET_VSG} ${TARGET_OSG2VSG})
    endif()
    set(VSGXCHANGE_INCLUDE_DIR ${OUTPUT_INCDIR} CACHE PATH "vsgXchange header directory" FORCE)
    set(VSGXCHANGE_LIBRARY ${OUTPUT_LIBDIR}/libvsgXchange${DEFAULT_LIB_SUFFIX} CACHE FILEPATH "vsgXchange Library" FORCE)
    set(VSGXCHANGE_LIBRARIES ${VSGXCHANGE_LIBRARY})
    set(VSGXCHANGE_FOUND TRUE CACHE BOOL "vsgXchange Library" FORCE)
    if (${BUILD_SHARED_LIBS})
        set(VSGXCHANGE_DEFINITIONS VSGXCHANGE_SHARED_LIBRARY)
    endif()
    message("In VSGXCHANGE Fallback setup")
endif()

add_subdirectory(applications)
