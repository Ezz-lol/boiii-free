#pragma once
#include <cstdint>
#include "../quake/vec.hpp"

namespace game {
namespace gfx {
namespace HLSL {

typedef vec2_t float2;
typedef vec3_t float3;
typedef vec4_t float4;

typedef vec2<float2> float2x2;
typedef vec3<float3> float3x3;
typedef vec4<float4> float4x4;

typedef vec2<uint32_t> uint2;
typedef vec3<uint32_t> uint3;
typedef vec4<uint32_t> uint4;

typedef vec2<uint2> uint2x2;
typedef vec3<uint3> uint3x3;
typedef vec4<uint4> uint4x4;
} // namespace HLSL
} // namespace gfx
} // namespace game