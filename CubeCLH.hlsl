#include "Common.hlsl"


[shader("closesthit")]void CubeHit(inout HitInfo payload,
                                       Attributes attrib)
{
    payload.colorAndDistance = float4(1.0f, attrib.bary, RayTCurrent());
}
