 
set(LOCAL_BINNAME canard_uts)

add_executable(${LOCAL_BINNAME} 
    main.c
)

target_link_libraries(${LOCAL_BINNAME} 
    ${CANARD_LIBNAME}
    ${DRV_SOCK_LIBNAME}
)

install(TARGETS ${LOCAL_BINNAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/static
)
