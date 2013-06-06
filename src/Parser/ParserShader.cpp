/*
 * ParserShader.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserShader.h"
#include "../HighlightSchema.h"

ParserShader::ParserShader()
{
}

ParserShader::~ParserShader()
{
}

int ParserShader::WordType(const string &name)
{
	if (name[0] == '#')
		return InMacro;
	if ((name == "varying") ||
		(name == "uniform") ||
		(name == "invariant") ||
		(name == "if") ||
		(name == "else") ||
		(name == "while") ||
		(name == "for") ||
		(name == "return") ||
		(name == "break") ||
		(name == "continue") ||
		(name == "const") ||
		(name == "<VertexShader>") ||
		(name == "</VertexShader>") ||
		(name == "<FragmentShader>") ||
		(name == "</FragmentShader>"))
		return InWordSpecial;
	if ((name == "void") ||
	    (name == "int") ||
		(name == "float") ||
		(name == "vec2") ||
		(name == "vec3") ||
		(name == "vec4") ||
		(name == "bool") ||
		(name == "mat2") ||
		(name == "mat3") ||
		(name == "mat4") ||
		(name == "mat2x2") ||
		(name == "mat2x3") ||
		(name == "mat2x4") ||
		(name == "mat3x2") ||
		(name == "mat3x3") ||
		(name == "mat3x4") ||
		(name == "mat4x2") ||
		(name == "mat4x3") ||
		(name == "mat4x4") ||
		(name == "sampler2D") ||
		(name == "samplerCube"))
		return InWordType;
	if ((name == "max") ||
		(name == "dot") ||
		(name == "pow") ||
		(name == "texture2D") ||
		(name == "textureCube") ||
		(name == "reflect") ||
		(name == "refract") ||
		(name == "normalize"))
		return InWordCompilerFunction;
	if ((name == "gl_Position") ||
		(name == "gl_TexCoord") ||
		(name == "gl_Vertex") ||
		(name == "gl_MultiTexCoord0") ||
		(name == "gl_MultiTexCoord1") ||
		(name == "gl_MultiTexCoord2") ||
		(name == "gl_MultiTexCoord3") ||
		(name == "gl_Normal") ||
		(name == "gl_NormalMatrix") ||
		(name == "gl_ModelViewMatrix") ||
		(name == "gl_ModelViewProjectionMatrix") ||
		(name == "gl_ModelViewMatrixInverse") ||
		(name == "gl_LightSource") ||
		(name == "gl_ModelViewProjectionMatrix") ||
		(name == "gl_FrontMaterial") ||
		(name == "gl_FrontColor") ||
		(name == "gl_Fog") ||
		(name == "gl_FogFragCoord") ||
		(name == "gl_FragCoord") ||
		(name == "gl_Color") ||
		(name == "gl_FragColor"))
		return InWordGameVariable;
	return -1;
}

void ParserShader::CreateTextColors(SourceView *sv, int first_line, int last_line)
{
	CreateTextColorsDefault(sv, first_line, last_line);
}

