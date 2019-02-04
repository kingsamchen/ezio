
set(KBASE_SOURCE_NAME KBase)
set(KBASE_SOURCE_VER v0.1.3)

# Content name is used by the module and in the lowercase.
string(TOLOWER "${KBASE_SOURCE_NAME}-${KBASE_SOURCE_VER}" KBASE_CONTENT_NAME)

# Source dir is where we put the source code of the dependency.
set(KBASE_SOURCE_DIR "${DEPS_SOURCE_DIR}/${KBASE_CONTENT_NAME}-src")

FetchContent_Declare(
  ${KBASE_CONTENT_NAME}
  GIT_REPOSITORY  https://github.com/kingsamchen/KBase.git
  GIT_TAG         ${KBASE_SOURCE_VER}
  GIT_SHALLOW     TRUE
  SOURCE_DIR      ${KBASE_SOURCE_DIR}
)

FetchContent_GetProperties(${KBASE_CONTENT_NAME})

if(NOT ${KBASE_CONTENT_NAME}_POPULATED)
  message(STATUS "Fetching dep: ${KBASE_CONTENT_NAME}")

  if(EXISTS "${KBASE_SOURCE_DIR}/CMakeLists.txt")
    message(STATUS "${KBASE_CONTENT_NAME} source dir is already ready; skip downloading.")
    set(FETCHCONTENT_SOURCE_DIR_KBASE ${KBASE_SOURCE_DIR})
  endif()

  FetchContent_Populate(${KBASE_CONTENT_NAME})

  # Set two module-defined variables.
  set(KBASE_CONTENT_SOURCE_DIR_VAR "${${KBASE_CONTENT_NAME}_SOURCE_DIR}")
  set(KBASE_CONTENT_BINARY_DIR_VAR "${${KBASE_CONTENT_NAME}_BINARY_DIR}")

  message(STATUS "${KBASE_CONTENT_NAME} source dir: ${KBASE_CONTENT_SOURCE_DIR_VAR}")
  message(STATUS "${KBASE_CONTENT_NAME} binary dir: ${KBASE_CONTENT_BINARY_DIR_VAR}")

  add_subdirectory(${KBASE_CONTENT_SOURCE_DIR_VAR} ${KBASE_CONTENT_BINARY_DIR_VAR})
endif()
