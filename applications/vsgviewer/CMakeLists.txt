set(APPLICATION_NAME vsgviewer)

set(SOURCES
    AnimationPath.cpp
    vsgviewer.cpp
)

add_executable(vsgviewer ${SOURCES})

if (TARGET_VSG)
    add_dependencies(${APPLICATION_NAME} ${TARGET_VSG})
endif()

if (TARGET_VSGXCHANGE)
    add_dependencies(${APPLICATION_NAME} ${TARGET_VSGXCHANGE})
endif()

target_link_libraries(vsgviewer vsg::vsg)

if (vsgXchange_FOUND)
    target_compile_definitions(vsgviewer PRIVATE USE_VSGXCHANGE)
    target_link_libraries(vsgviewer vsgXchange::vsgXchange)
endif()
