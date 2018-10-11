

cmake_minimum_required(VERSION 2.8)

project(gtest_download NONE)

include(ExternalProject)

# Configure gtest as an external project
ExternalProject_Add(project_gtest
    GIT_REPOSITORY      https://github.com/google/googletest.git
    GIT_TAG             release-1.8.0
    SOURCE_DIR          ${GTEST_DIR_SOURCE}
    BINARY_DIR          ${GTEST_DIR_BINARY}
    CONFIGURE_COMMAND   ""
    BUILD_COMMAND       ""
    INSTALL_COMMAND     ""
    TEST_COMMAND        ""
)
