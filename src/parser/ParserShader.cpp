/*
 * ParserShader.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "ParserShader.h"
#include "../HighlightScheme.h"

ParserShader::ParserShader() : Parser("Shader") {
	macro_begin = "#";
	line_comment_begin = "//";
	multi_comment_begin = "/*";
	multi_comment_end = "*/";
	keywords.add("varying");
	keywords.add("uniform");
	keywords.add("invariant");
	keywords.add("writeonly");
	keywords.add("in");
	keywords.add("out");
	keywords.add("layout");
	keywords.add("readonly");
	keywords.add("require");
	keywords.add("shared");
	keywords.add("triangles");
	keywords.add("lines");
	keywords.add("points");
	keywords.add("triangle_strip");
	keywords.add("max_vertices");
	keywords.add("struct");
	keywords.add("if");
	keywords.add("else");
	keywords.add("while");
	keywords.add("for");
	keywords.add("return");
	keywords.add("break");
	keywords.add("continue");
	keywords.add("const");
	keywords.add("discard");
	/*keywords.add("<VertexShader>");
	keywords.add("</VertexShader>");
	keywords.add("<FragmentShader>");
	keywords.add("</FragmentShader>");*/
	keywords.add("Layout");
	keywords.add("VertexShader");
	keywords.add("FragmentShader");
	keywords.add("GeometryShader");
	keywords.add("ComputeShader");
	keywords.add("TessComputeShader");
	keywords.add("TessEvaluationShader");
	keywords.add("RayGenShader");
	keywords.add("RayMissShader");
	keywords.add("RayClosestHitShader");
	keywords.add("RayAnyHitShader");
	keywords.add("Module");
	types.add("void");
	types.add("int");
	types.add("uint");
	types.add("float");
	types.add("vec2");
	types.add("vec3");
	types.add("vec4");
	types.add("ivec2");
	types.add("ivec3");
	types.add("ivec4");
	types.add("uvec2");
	types.add("uvec3");
	types.add("uvec4");
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
	constants.add("true");
	constants.add("false");
	compiler_functions.add("max");
	compiler_functions.add("min");
	compiler_functions.add("dot");
	compiler_functions.add("cross");
	compiler_functions.add("pow");
	compiler_functions.add("sqrt");
	compiler_functions.add("fract");
	compiler_functions.add("abs");
	compiler_functions.add("exp");
	compiler_functions.add("sin");
	compiler_functions.add("cos");
	compiler_functions.add("tan");
	compiler_functions.add("step");
	compiler_functions.add("clamp");
	compiler_functions.add("length");
	compiler_functions.add("smoothstep");
	compiler_functions.add("texture");
	compiler_functions.add("texture2D");
	compiler_functions.add("textureCube");
	compiler_functions.add("textureLod");
	compiler_functions.add("textureSize");
	compiler_functions.add("textureGrad");
	compiler_functions.add("texelFetch");
	compiler_functions.add("imageStore");
	compiler_functions.add("imageLoad");
	compiler_functions.add("imageSize");
	compiler_functions.add("reflect");
	compiler_functions.add("refract");
	compiler_functions.add("normalize");
	compiler_functions.add("inverse");
	compiler_functions.add("transpose");
	compiler_functions.add("EmitVertex");
	compiler_functions.add("EndPrimitive");
	compiler_functions.add("atomicAdd");
	compiler_functions.add("atomicAnd");
	compiler_functions.add("atomicOr");
	compiler_functions.add("atomicMin");
	compiler_functions.add("atomicMax");
	compiler_functions.add("atomicSwap");
	compiler_functions.add("atomicExchange");
	compiler_functions.add("barrier");
	compiler_functions.add("memoryBarrier");
	compiler_functions.add("groupMemoryBarrier");
	compiler_functions.add("memoryBarrierBuffer");
	compiler_functions.add("memoryBarrierImage");
	compiler_functions.add("memoryBarrierShared");
	compiler_functions.add("memoryBarrierAtomicCounter");
	global_variables.add("gl_Position");
	global_variables.add("gl_in");
	// <Layout>...
	global_variables.add("bindings");
	global_variables.add("pushsize");
	global_variables.add("input");
	global_variables.add("topology");
	global_variables.add("version");
	global_variables.add("name");
	global_variables.add("buffer");
	global_variables.add("image");
	global_variables.add("sampler");
	// layout(x=y...)
	global_variables.add("location");
	global_variables.add("binding");
	global_variables.add("set");
	global_variables.add("component");
	global_variables.add("push_constant");
	global_variables.add("local_size_x");
	global_variables.add("local_size_y");
	global_variables.add("std430");
	global_variables.add("std140");
	global_variables.add("rgba8");
	global_variables.add("rgba8i");
	global_variables.add("rgba8ui");
	global_variables.add("rgba16f");
	global_variables.add("rgba32f");
	global_variables.add("r8");
	global_variables.add("r8i");
	global_variables.add("r8ui");
	global_variables.add("r16f");
	global_variables.add("r32f");

	global_variables.add("gl_TexCoord");
	global_variables.add("gl_Vertex");
	global_variables.add("gl_MultiTexCoord0");
	global_variables.add("gl_MultiTexCoord1");
	global_variables.add("gl_MultiTexCoord2");
	global_variables.add("gl_MultiTexCoord3");
	global_variables.add("gl_Normal");
	global_variables.add("gl_NormalMatrix");
	global_variables.add("gl_ModelViewMatrix");
	global_variables.add("gl_ModelViewProjectionMatrix");
	global_variables.add("gl_ModelViewMatrixInverse");
	global_variables.add("gl_LightSource");
	global_variables.add("gl_ModelViewProjectionMatrix");
	global_variables.add("gl_FrontMaterial");
	global_variables.add("gl_FrontColor");
	global_variables.add("gl_Fog");
	global_variables.add("gl_FogFragCoord");
	global_variables.add("gl_FragCoord");
	global_variables.add("gl_Color");
	global_variables.add("gl_FragColor");
	global_variables.add("gl_FragDepth");
	global_variables.add("gl_GlobalInvocationID");
	global_variables.add("gl_NumWorkGroups");
	global_variables.add("gl_WorkGroupID");
	global_variables.add("gl_WorkGroupSize");
	global_variables.add("gl_LocalInvocationID");
	global_variables.add("gl_PrimitiveID");
	global_variables.add("gl_InstanceID");
	global_variables.add("gl_InstanceIndex");
	global_variables.add("gl_LocalInvocationID");
	global_variables.add("gl_LocalInvocationIndex");

	// rtx
	keywords.add("GL_NV_ray_tracing");
	keywords.add("GL_GOOGLE_include_directive");
	keywords.add("GL_EXT_nonuniform_qualifier");
	types.add("accelerationStructureNV");
	keywords.add("callableDataNV");
	keywords.add("callableDataInNV");
	keywords.add("rayPayloadNV");
	keywords.add("rayPayloadInNV");
	keywords.add("hitAttributeNV");
	global_variables.add("gl_LaunchIDNV");
	global_variables.add("gl_LaunchSizeNV");
	global_variables.add("gl_RayFlagsNoneNV");
	global_variables.add("gl_InstanceCustomIndexNV");
	global_variables.add("gl_HitTNV");
	global_variables.add("gl_RayFlagsNoneNV");
	global_variables.add("gl_RayFlagsOpaqueNV");
	global_variables.add("gl_RayFlagsNoOpaqueNV");
	global_variables.add("gl_RayFlagsTerminateOnFirstHitNV");
	global_variables.add("gl_RayFlagsSkipClosestHitShaderNV");
	global_variables.add("gl_RayFlagsCullBackFacingTrianglesNV");
	global_variables.add("gl_RayFlagsCullFrontFacingTrianglesNV");
	global_variables.add("gl_RayFlagsCullOpaqueNV");
	global_variables.add("gl_RayFlagsCullNoOpaqueNV");
	global_variables.add("gl_WorldRayOriginNV");
	global_variables.add("gl_WorldRayDirectionNV");
	global_variables.add("gl_ObjectRayOriginNV");
	global_variables.add("gl_ObjectRayDirectionNV");
	global_variables.add("gl_RayTminNV");
	global_variables.add("gl_RayTmaxNV");
	global_variables.add("gl_IncomingRayFlagsNV");
	global_variables.add("gl_HitKindNV");
	global_variables.add("gl_WorldToObjectNV");
	global_variables.add("gl_ObjectToWorldNV");
	compiler_functions.add("traceNV");
	compiler_functions.add("reportIntersectionNV");
	compiler_functions.add("ignoreIntersectionNV");
	compiler_functions.add("terminateRayNV");
	compiler_functions.add("executeCallableNV");
	compiler_functions.add("nonuniformEXT");
}

void ParserShader::CreateTextColors(SourceView *sv, int first_line, int last_line) {
	CreateTextColorsDefault(sv, first_line, last_line);
}

