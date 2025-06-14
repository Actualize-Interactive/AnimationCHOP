
in int isKnob;

out Vertex
{
	vec4 color;
	vec4 customAttrib0;
} oVert;

void main()
{

	// First deform the vertex and normal
	// TDDeform always returns values in world space
	//mat3 rot = mat3(TDInstanceMat());
	vec3 pos = P;
	vec4 customAttrib0 = TDInstanceCustomAttrib0();
	// if (isKnob == 1) {
	// 	pos.xy = customAttrib0.xy;
	// }

	vec4 worldSpacePos = TDDeform(pos);
		if (isKnob == 1) {
		worldSpacePos.xy = customAttrib0.xy;
	}
	//vec4 worldSpacePos = TDInstanceMat() * vec4(pos, 1.0);

	gl_Position = TDWorldToProj(worldSpacePos);

	oVert.customAttrib0 = customAttrib0;

	// This is here to ensure we only execute lighting etc. code
	// when we need it. If picking is active we don't need lighting, so
	// this entire block of code will be ommited from the compile.
	// The TD_PICKING_ACTIVE define will be set automatically when
	// picking is active.
#ifndef TD_PICKING_ACTIVE
	oVert.color = TDInstanceColor(Cd);

#else // TD_PICKING_ACTIVE
	// This will automatically write out the nessessary values
	// for this shader to work with picking.
	// See the documentation if you want to write custom values for picking.
	TDWritePickingValues();

#endif // TD_PICKING_ACTIVE
}
