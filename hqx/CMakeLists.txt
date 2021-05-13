
add_library (hqx SHARED hq2x.c hq3x.c hq4x.c init.c)
target_compile_definitions(hqx PUBLIC DLL_EXPORT)


target_include_directories (hqx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})