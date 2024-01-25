include_guard(GLOBAL)


include_directories(. third_party third_party/spdlog/include third_party/boost third_party/mbedtls/include)


add_definitions(-D_WIN32_WINNT=0x0601)
add_definitions(-DBOOST_JSON_NO_LIB)
add_definitions(-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)


add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/mbedtls/library)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/buried_config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/buried_config.h)


set(DB_SRC third_party/sqlite/sqlite3.c database/database.cpp)

set(BURIED_SRC 
    ${DB_SRC}
	context/context.cpp
	common/common_service.cpp
	crypt/crypt.cpp
	report/buried_report.cpp
	report/http_report.cpp
	buried.cpp
	buried_core.cpp
	
    )

set(MODULE_NAME "Buried")

message(STATUS "libs ${LIBS}")

if(BUILD_BURIED_STATIC_LIBS)
	add_library(${MODULE_NAME}_static STATIC ${BURIED_SRC})
	target_link_libraries(${MODULE_NAME}_static PRIVATE ${LIBS} spdlog mbedtls_static)
endif()



if(BUILD_BURIED_SHARED_LIBS)
	add_library(${MODULE_NAME}_shared SHARED ${BURIED_SRC})
	target_link_libraries(${MODULE_NAME}_shared PRIVATE ${LIBS} spdlog mbedtls_static)
endif()
