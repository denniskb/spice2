#foreach(sample brunel brunel+ vogels)
foreach(sample brunel+)
	add_executable(${sample}
	matplot.h
	matplot.cpp
	${sample}.cpp)
	
	target_compile_options(${sample} PRIVATE ${warning_flags} ${math_flags})
	target_include_directories(${sample} PRIVATE ${CMAKE_SOURCE_DIR}/spice/include ${CMAKE_SOURCE_DIR}/third_party/matplot/source)
	target_link_libraries(${sample} PRIVATE spice matplot)
endforeach()