cmake_minimum_required(VERSION 3.16)
project(wms_gui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Widgets PrintSupport REQUIRED)
find_package(Qt6 COMPONENTS Charts QUIET)
if (TARGET Qt6::Charts)
    set(WMS_GUI_HAS_CHARTS ON)
else()
    set(WMS_GUI_HAS_CHARTS OFF)
endif()

# ── Backend source files ─────────────────────────────────────────────────────
set(BACKEND_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src")
set(VENDOR_DIR      "${CMAKE_CURRENT_SOURCE_DIR}/../vendor")

# SQLiteCpp static library (vendored)
add_subdirectory("${VENDOR_DIR}/SQLiteCpp" "${CMAKE_BINARY_DIR}/SQLiteCpp_build" EXCLUDE_FROM_ALL)

# All backend .cpp files the GUI needs
set(BACKEND_SOURCES
    "${BACKEND_SRC_DIR}/models/Item.cpp"
    "${BACKEND_SRC_DIR}/models/Inventory.cpp"
    "${BACKEND_SRC_DIR}/models/Customer.cpp"
    "${BACKEND_SRC_DIR}/storage/Storage.cpp"
    "${BACKEND_SRC_DIR}/storage/Receipt.cpp"
    "${BACKEND_SRC_DIR}/storage/CustomerStorage.cpp"
    "${BACKEND_SRC_DIR}/controllers/WmsControllers.cpp"
    "${BACKEND_SRC_DIR}/reports/ReportEngine.cpp"
    "${BACKEND_SRC_DIR}/output/output.cpp"
)

# ── GUI executable ────────────────────────────────────────────────────────────
add_executable(wms_gui
    main.cpp
    main.h
    main.ui
    ${BACKEND_SOURCES}
)

target_include_directories(wms_gui PRIVATE
    "${BACKEND_SRC_DIR}"
    "${VENDOR_DIR}/SQLiteCpp/include"
    "${VENDOR_DIR}/SQLiteCpp/sqlite3"
)

target_link_libraries(wms_gui
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::PrintSupport
        SQLiteCpp
)

if (WMS_GUI_HAS_CHARTS)
    target_compile_definitions(wms_gui PRIVATE WMS_GUI_HAS_CHARTS=1)
    target_link_libraries(wms_gui PRIVATE Qt6::Charts)
endif()