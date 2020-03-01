/*
 * ParserShader.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserShader.h"
#include "../HighlightScheme.h"

ParserShader::ParserShader() : Parser("Sader") {
	macro_begin = "#";
	line_comment_begin = "//";
	multi_comment_begin = "/*";
	multi_comment_end = "*/";
	special_words.add("varying");
	special_words.add("uniform");
	special_words.add("invariant");
	special_words.add("writeonly");
	special_words.add("in");
	special_words.add("out");
	special_words.add("layout");
	special_words.add("push_constant");
	special_words.add("buffer");
	special_words.add("sampler");
	special_words.add("triangles");
	special_words.add("lines");
	special_words.add("points");
	special_words.add("triangle_strip");
	special_words.add("max_vertices");
	special_words.add("local_size_x");
	special_words.add("local_size_y");
	special_words.add("struct");
	special_words.add("if");
	special_words.add("else");
	special_words.add("while");
	special_words.add("for");
	special_words.add("return");
	special_words.add("break");
	special_words.add("continue");
	special_words.add("const");
	/*special_words.add("<VertexShader>");
	special_words.add("</VertexShader>");
	special_words.add("<FragmentShader>");
	special_words.add("</FragmentShader>");*/
	special_words.add("Layout");
	special_words.add("VertexShader");
	special_words.add("FragmentShader");
	special_words.add("GeometryShader");
	special_words.add("ComputeShader");
	special_words.add("TessComputeShader");
	special_words.add("TessEvaluationShader");
	types.add("void");
	types.add("int");
	types.add("float");
	types.add("vec2");
	types.add("vec3");
	types.add("vec4");
	types.add("ivec2");
	types.add("ivec3");
	types.add("ivec4");
	types.add("bool");
	types.add("mat2");
	types.add("mat3");
	types.add("mat4");
	types.add("mat2x2");
	types.add("mat2x3");
	types.add("mat2x4");
	types.add("mat3x2");
	types.add("mat3x3");
	types.add("mat4x2");
	types.add("mat4x3");
	types.add("mat4x4");
	types.add("sampler2D");
	types.add("sampler2DShadow");
	types.add("samplerCube");
	types.add("image2D");
	compiler_functions.add("max");
	compiler_functions.add("min");
	compiler_functions.add("dot");
	compiler_functions.add("cross");
	compiler_functions.add("pow");
	compiler_functions.add("abs");
	compiler_functions.add("exp");
	compiler_functions.add("sin");
	compiler_functions.add("cos");
	compiler_functions.add("tan");
	compiler_functions.add("step");
	compiler_functions.add("smoothstep");
	compiler_functions.add("texture");
	compiler_functions.add("texture2D");
	compiler_functions.add("textureCube");
	compiler_functions.add("imageStore");
	compiler_functions.add("imageLoad");
	compiler_functions.add("reflect");
	compiler_functions.add("refract");
	compiler_functions.add("normalize");
	compiler_functions.add("inverse");
	compiler_functions.add("transpose");
	compiler_functions.add("EmitVertex");
	compiler_functions.add("EndPrimitive");
	globals.add("gl_Position");
	globals.add("gl_in");
	// <Layout>...
	globals.add("bindings");
	globals.add("pushsize");
	globals.add("input");
	globals.add("topology");
	// layout(x=y...)
	globals.add("location");
	globals.add("binding");
	globals.add("set");

	globals.add("gl_TexCoord");
	globals.add("gl_Vertex");
	globals.add("gl_MultiTexCoord0");
	globals.add("gl_MultiTexCoord1");
	globals.add("gl_MultiTexCoord2");
	globals.add("gl_MultiTexCoord3");
	globals.add("gl_Normal");
	globals.add("gl_NormalMatrix");
	globals.add("gl_ModelViewMatrix");
	globals.add("gl_ModelViewProjectionMatrix");
	globals.add("gl_ModelViewMatrixInverse");
	globals.add("gl_LightSource");
	globals.add("gl_ModelViewProjectionMatrix");
	globals.add("gl_FrontMaterial");
	globals.add("gl_FrontColor");
	globals.add("gl_Fog");
	globals.add("gl_FogFragCoord");
	globals.add("gl_FragCoord");
	globals.add("gl_Color");
	globals.add("gl_FragColor");
	globals.add("gl_FragDepth");
	globals.add("gl_GlobalInvocationID");
	globals.add("gl_NumWorkGroups");
	globals.add("gl_WorkGroupID");
	globals.add("gl_WorkGroupSize");
	globals.add("gl_LocalInvocationID");
}

void ParserShader::CreateTextColors(SourceView *sv, int first_line, int last_line) {
	CreateTextColorsDefault(sv, first_line, last_line);
}

