set(INPUTS_PATH ${CMAKE_CURRENT_LIST_DIR}/)
set(GEN_OUT_PATH ${GEN_ROOT}/multipass-3/)
set(INPUTS_LIST ${INPUTS_PATH}D.h ${INPUTS_PATH}C.h ${INPUTS_PATH}B.h ${INPUTS_PATH}A.h)
list(APPEND TEST_FILES ${INPUTS_PATH}multipass-3.cpp)
include(${CMAKE_CURRENT_LIST_DIR}/../setup-case.cmake)
