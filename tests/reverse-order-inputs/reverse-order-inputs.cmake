set(INPUTS_PATH ${CMAKE_CURRENT_LIST_DIR}/)
set(GEN_OUT_PATH ${GEN_ROOT}/reverse-order-inputs/)
set(INPUTS_LIST ${INPUTS_PATH}5.h ${INPUTS_PATH}4.h ${INPUTS_PATH}3.h ${INPUTS_PATH}2.h ${INPUTS_PATH}1.h)
list(APPEND TEST_FILES ${INPUTS_PATH}reverse-order-inputs.cpp)
include(${CMAKE_CURRENT_LIST_DIR}/../setup-case.cmake)
