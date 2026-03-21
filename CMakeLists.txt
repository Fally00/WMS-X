cmake_minimum_required(VERSION 3.16)
project(wms LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")

# SQLiteCpp
add_subdirectory("${VENDOR_DIR}/SQLiteCpp" "${CMAKE_BINARY_DIR}/SQLiteCpp_build" EXCLUDE_FROM_ALL)

add_executable(wms
    "${SRC_DIR}/main.cpp"
    "${SRC_DIR}/models/Item.cpp"
    "${SRC_DIR}/models/Inventory.cpp"
    "${SRC_DIR}/storage/Storage.cpp"
    "${SRC_DIR}/storage/Receipt.cpp"
    "${SRC_DIR}/controllers/WmsControllers.cpp"
    "${SRC_DIR}/output/output.cpp"
    "${SRC_DIR}/cli/cli.cpp"
)

target_include_directories(wms PRIVATE
    "${SRC_DIR}"
    "${VENDOR_DIR}/SQLiteCpp/include"
    "${VENDOR_DIR}/SQLiteCpp/sqlite3"
)

target_link_libraries(wms PRIVATE SQLiteCpp pthread)