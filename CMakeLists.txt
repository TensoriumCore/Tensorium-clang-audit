cmake_minimum_required(VERSION 3.20)

project(tensorium-clang-audit LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_language(C)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "LLVM include dirs: ${LLVM_INCLUDE_DIRS}")
message(STATUS "Clang include dirs: ${CLANG_INCLUDE_DIRS}")

add_library(TensoriumClangAudit SHARED
  src/Plugin.cpp
  src/Visitor.cpp
)

target_include_directories(TensoriumClangAudit
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(TensoriumClangAudit
  PRIVATE
    ${LLVM_DEFINITIONS}
)

target_link_libraries(TensoriumClangAudit
  PRIVATE
    clangAST
    clangBasic
    clangFrontend
    clangSerialization
)

if(APPLE)
  set_target_properties(TensoriumClangAudit PROPERTIES
    SUFFIX ".so"
  )
endif()
