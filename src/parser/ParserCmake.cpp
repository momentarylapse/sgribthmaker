/*
 * ParserCmake.cpp
 *
 *  Created on: 25.06.2013
 *      Author: michi
 */

#include "ParserCmake.h"
#include "../HighlightScheme.h"

ParserCmake::ParserCmake() : Parser("Python") {
	line_comment_begin = "#";
	string_sub_begin = "${";
	string_sub_end = "}";
	keywords.add("if");
	keywords.add("else");
	keywords.add("endif");
	keywords.add("macro");
	keywords.add("endmacro");
	operator_functions.add("AND");
	operator_functions.add("GET");
	operator_functions.add("MATCH");
	operator_functions.add("NOT");
	operator_functions.add("OR");
	operator_functions.add("STREQUAL");
	types.add("APPEND");
	types.add("BOOL");
	types.add("CACHE");
	types.add("CODE");
	types.add("DESTINATION");
	types.add("DIRECTORIES");
	types.add("DIRECTORY");
	types.add("FILES");
	types.add("FORCE");
	types.add("INSTALL_PREFIX");
	types.add("INTERFACE");
	types.add("NO_SOURCE_PERMISSIONS");
	types.add("ON");
	types.add("OFF");
	types.add("PUBLIC");
	types.add("PRIVATE");
	types.add("REQUIRED");
	types.add("SEND_ERROR");
	types.add("STATUS");
	types.add("STRING");
	types.add("TARGETS");
	types.add("VERSION");
	types.add("WARNING");
	global_variables.add("APPLE");
	global_variables.add("CMAKE_CXX_STANDARD");
	global_variables.add("CMAKE_CXX_STANDARD_REQUIRED");
	global_variables.add("CMAKE_BUILD_TYPE");
	global_variables.add("CMAKE_SYSTEM");
	global_variables.add("CMAKE_SYSTEM_VERSION");
	global_variables.add("MSVC");
	global_variables.add("THREADS_PREFER_PTHREAD_FLAG");
	global_variables.add("UNIX");
	global_variables.add("WIN32");
	compiler_functions.add("add_executable");
	compiler_functions.add("add_library");
	compiler_functions.add("cmake_minimum_required");
	compiler_functions.add("configure_file");
	compiler_functions.add("copy_if_different");
	compiler_functions.add("execute_process");
	compiler_functions.add("find_library");
	compiler_functions.add("find_package");
	compiler_functions.add("find_program");
	compiler_functions.add("include");
	compiler_functions.add("install");
	compiler_functions.add("list");
	compiler_functions.add("message");
	compiler_functions.add("pkg_check_modules");
	compiler_functions.add("project");
	compiler_functions.add("set");
	compiler_functions.add("target_compile_options");
	compiler_functions.add("target_compile_definitions");
	compiler_functions.add("target_include_directories");
	compiler_functions.add("target_link_directories");
	compiler_functions.add("target_link_libraries");

	constants.add("True");
	constants.add("true");
	constants.add("False");
	constants.add("false");
}

void ParserCmake::CreateTextColors(SourceView *sv, int first_line, int last_line) {
	CreateTextColorsDefault(sv, first_line, last_line);
}

