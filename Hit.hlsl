#include "Common.hlsli"



[shader("closesthit")] void ClosestHit(inout RayPayload payload,
                                       Attributes attrib) {
  payload.colorAndDistance = float4((attrib.bary), 1.0f, RayTCurrent());
 
}
