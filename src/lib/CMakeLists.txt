 
add_library(${CANARD_LIBNAME} STATIC 
    crcAdd.c
    crcAddByte.c
    crcAddSignature.c
    canard.c
)

# set_target_properties(${CANARD_LIBNAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

install(TARGETS ${CANARD_LIBNAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
