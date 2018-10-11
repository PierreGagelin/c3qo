

cmake_minimum_required(VERSION 2.8)

project(jsmn_download NONE)

include(ExternalProject)

# Configure jsmn as an external project
ExternalProject_Add(project_jsmn
    GIT_REPOSITORY      https://github.com/zserge/jsmn.git
    GIT_TAG             18e9fe42cbfe21d65076f5c77ae2be379ad1270f
    SOURCE_DIR          ${JSMN_DIR_SOURCE}
    BINARY_DIR          ${JSMN_DIR_BINARY}
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
    TEST_COMMAND        ""
)
