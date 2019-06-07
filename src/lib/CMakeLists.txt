
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(${CANARD_LIBNAME} STATIC 
    # CRC
    crcAdd.c
    crcAddByte.c
    crcAddSignature.c
    
    # allocator
    allocateBlock.c
    freeBlock.c
    initPoolAllocator.c
    
    convert_float.c
    canard.c
)

# set_target_properties(${CANARD_LIBNAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

install(TARGETS ${CANARD_LIBNAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
