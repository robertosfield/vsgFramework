SET(APPLICATION_NAME vsgvalues)

INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/include/
)

SET(SOURCES
    vsgvalues.cpp
)

ADD_EXECUTABLE(${APPLICATION_NAME} ${HEADERS} ${SOURCES})

if (TARGET_OSG OR TARGET_VSG)
    add_dependencies(${APPLICATION_NAME} ${TARGET_VSG})
endif()

TARGET_LINK_LIBRARIES(${APPLICATION_NAME} vsg::vsg)
