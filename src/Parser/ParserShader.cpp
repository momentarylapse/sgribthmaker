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
	keywords.add("push_constant");
	keywords.add("buffer");
	keywords.add("sampler");
	keywords.add("triangles");
	keywords.add("lines");
	keywords.add("points");
	keywords.add("triangle_strip");
	keywords.add("max_vertices");
	keywords.add("local_size_x");
	keywords.add("local_size_y");
	keywords.add("struct");
	keywords.add("if");
	keywords.add("else");
	keywords.add("while");
	keywords.add("for");
	keywords.add("return");
	keywords.add("break");
	keywords.add("continue");
	keywords.add("const");
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
	globals.add("gl_Position");
	globals.add("gl_in");
	// <Layout>...
	globals.add("bindings");
	globals.add("pushsize");
	globals.add("input");
	globals.add("topology");
	globals.add("version");
	globals.add("name");
	// layout(x=y...)
	globals.add("location");
	globals.add("binding");
	globals.add("set");
	globals.add("component");
	globals.add("std430");
	globals.add("std140");
	globals.add("rgba8");
	globals.add("rgba8i");
	globals.add("rgba8ui");
	globals.add("rgba16f");
	globals.add("rgba32f");
	globals.add("r8");
	globals.add("r8i");
	globals.add("r8ui");
	globals.add("r16f");
	globals.add("r32f");

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
	globals.add("gl_PrimitiveID");
	globals.add("gl_InstanceID");
	globals.add("gl_LocalInvocationID");
	globals.add("gl_LocalInvocationIndex");

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
	globals.add("gl_LaunchIDNV");
	globals.add("gl_LaunchSizeNV");
	globals.add("gl_RayFlagsNoneNV");
	globals.add("gl_InstanceCustomIndexNV");
	globals.add("gl_HitTNV");
	globals.add("gl_RayFlagsNoneNV");
	globals.add("gl_RayFlagsOpaqueNV");
	globals.add("gl_RayFlagsNoOpaqueNV");
	globals.add("gl_RayFlagsTerminateOnFirstHitNV");
	globals.add("gl_RayFlagsSkipClosestHitShaderNV");
	globals.add("gl_RayFlagsCullBackFacingTrianglesNV");
	globals.add("gl_RayFlagsCullFrontFacingTrianglesNV");
	globals.add("gl_RayFlagsCullOpaqueNV");
	globals.add("gl_RayFlagsCullNoOpaqueNV");
	globals.add("gl_WorldRayOriginNV");
	globals.add("gl_WorldRayDirectionNV");
	globals.add("gl_ObjectRayOriginNV");
	globals.add("gl_ObjectRayDirectionNV");
	globals.add("gl_RayTminNV");
	globals.add("gl_RayTmaxNV");
	globals.add("gl_IncomingRayFlagsNV");
	globals.add("gl_HitKindNV");
	globals.add("gl_WorldToObjectNV");
	globals.add("gl_ObjectToWorldNV");
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

