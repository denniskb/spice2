foreach(sample brunel brunel+ external_input ping_pong sssp vogels)
	add_executable(${sample}
	matplot.h
	matplot.cpp
	${sample}.cpp)
	
	target_compile_options(${sample} PRIVATE ${spice_warning_flags} ${spice_math_flags})
	target_link_libraries(${sample} PRIVATE spice)

	if (spice_use_matplot)
		target_compile_definitions(${sample} PRIVATE SPICE_USE_MATPLOT)
		target_link_libraries(${sample} PRIVATE matplot)
	endif()
endforeach()