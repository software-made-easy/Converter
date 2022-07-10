set(MD4C_FILES
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/entity.h
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/entity.c
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/md4c.h
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/md4c.c
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/md4c-html.h
    ${CMAKE_CURRENT_LIST_DIR}/md4c/src/md4c-html.c
)

add_library(md4c-html STATIC ${MD4C_FILES})

include_directories(${CMAKE_CURRENT_LIST_DIR}/md4c/src)

set(3rdparty
    md4c-html
)
