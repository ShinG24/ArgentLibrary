#include "Common.hlsl"



[shader("closesthit")] void ClosestHit(inout HitInfo payload,
                                       Attributes attrib) {
  payload.colorAndDistance = float4((attrib.bary), 1.0f, RayTCurrent());
 
}
