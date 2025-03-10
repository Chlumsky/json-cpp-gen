
set(CASE_GENERATED_FILES ${GEN_OUT_PATH}Parser.h ${GEN_OUT_PATH}Parser.cpp ${GEN_OUT_PATH}Serializer.h ${GEN_OUT_PATH}Serializer.cpp)
configure_file(${INPUTS_PATH}config.json ${GEN_OUT_PATH}config.json)
list(APPEND GENERATED_FILES ${CASE_GENERATED_FILES})
list(APPEND TEST_FILES ${INPUTS_LIST})

add_custom_command(
    OUTPUT ${CASE_GENERATED_FILES}
    COMMAND json-cpp-gen ${GEN_OUT_PATH}config.json
    DEPENDS json-cpp-gen ${INPUTS_PATH}config.json ${INPUTS_LIST}
)
