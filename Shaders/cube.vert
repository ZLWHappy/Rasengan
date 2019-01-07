#version 330 compatibility
uniform sampler2D instanceMatrixTexture;
uniform sampler2D instanceColorTexture;
uniform sampler2D instanceAxisTexture;
uniform int width;
uniform float t;

out vec4 vs_gs_color;
out vec3 vs_gs_rot_axis;
out float vs_gs_theta;

mat4 getRotateMatrix(vec3 v1, vec3 v2, float theta)
{
	mat4 ret;

	float a = v1.x;
	float b = v1.y;
	float c = v1.z;

	vec3 p = normalize(v2 - v1);
	float u = p.x;
	float v = p.y;
	float w = p.z;

	float uu = u * u;
    float uv = u * v;
    float uw = u * w;
    float vv = v * v;
    float vw = v * w;
    float ww = w * w;
    float au = a * u;
    float av = a * v;
    float aw = a * w;
    float bu = b * u;
    float bv = b * v;
    float bw = b * w;
    float cu = c * u;
    float cv = c * v;
    float cw = c * w;

	float costheta = cos(theta);
    float sintheta = sin(theta);

	ret[0][0] = uu + (vv + ww) * costheta;
    ret[0][1] = uv * (1 - costheta) + w * sintheta;
    ret[0][2] = uw * (1 - costheta) - v * sintheta;
    ret[0][3] = 0;

    ret[1][0] = uv * (1 - costheta) - w * sintheta;
    ret[1][1] = vv + (uu + ww) * costheta;
    ret[1][2] = vw * (1 - costheta) + u * sintheta;
    ret[1][3] = 0;

    ret[2][0] = uw * (1 - costheta) + v * sintheta;
    ret[2][1] = vw * (1 - costheta) - u * sintheta;
    ret[2][2] = ww + (uu + vv) * costheta;
    ret[2][3] = 0;

    ret[3][0] = (a * (vv + ww) - u * (bv + cw)) * (1 - costheta) + (bw - cv) * sintheta;
    ret[3][1] = (b * (uu + ww) - v * (au + cw)) * (1 - costheta) + (cu - aw) * sintheta;
    ret[3][2] = (c * (uu + vv) - w * (au + bv)) * (1 - costheta) + (av - bu) * sintheta;
    ret[3][3] = 1;

	return ret;
}
void main()
{
	vec4 texValue = texelFetch(instanceMatrixTexture,ivec2(gl_InstanceID % width, gl_InstanceID / width),0);
	vec4 pos = gl_Vertex + vec4(texValue.x,texValue.y,texValue.z, 1.0);

	gl_Position = pos;
	
	vec4 texValue3 = texelFetch(instanceAxisTexture,ivec2(gl_InstanceID % width, gl_InstanceID / width),0);
	vec3 rotateAxis = texValue3.xyz;
	vs_gs_rot_axis = rotateAxis;
	vs_gs_theta = t + rotateAxis.y;
	
	// vec3 origin = vec3(0.0,0.0,0.0);
	// mat4 rotateMatrix = getRotateMatrix(origin, rotateAxis, t * 0.0001);
	// pos = rotateMatrix * pos;



	// gl_Position = gl_ModelViewProjectionMatrix * pos;
	// vec4 texValue2 = texelFetch(instanceColorTexture,ivec2(gl_InstanceID % width, gl_InstanceID / width),0);
	// vs_gs_color = vec4(texValue2.x,texValue2.y,texValue2.z, 1.0);
}