SET(APPLICATION_NAME vsgallocator)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/
)

SET(SOURCES
    vsgallocator.cpp
)

ADD_EXECUTABLE(${APPLICATION_NAME} ${HEADERS} ${SOURCES})

if (TARGET_VSG)
    add_dependencies(${APPLICATION_NAME} ${TARGET_VSG})
endif()

TARGET_LINK_LIBRARIES(${APPLICATION_NAME} vsg::vsg)
