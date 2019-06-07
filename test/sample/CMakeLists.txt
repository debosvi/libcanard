 
set(LOCAL_BINNAME sample_send)

# Demo application
exec_program("git"
             ${CMAKE_CURRENT_SOURCE_DIR}
             ARGS "rev-parse --short=8 HEAD"
             OUTPUT_VARIABLE GIT_HASH)
                           
                           
add_executable(${LOCAL_BINNAME} 
    demo_send.c
)

target_compile_definitions(${LOCAL_BINNAME} 
    PUBLIC GIT_HASH=0x${GIT_HASH})
    
# set_target_properties(${LOCAL_BINNAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

target_link_libraries(${LOCAL_BINNAME} 
    ${CANARD_LIBNAME}
    ${DRV_SOCK_LIBNAME}
    skarnet
)

install(TARGETS ${LOCAL_BINNAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
