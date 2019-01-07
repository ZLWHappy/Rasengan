#version 430 compatibility
layout (triangles) in;
layout (triangle_strip, max_vertices = 120) out;

in vec3 vs_gs_rot_axis[];
in float vs_gs_theta[];
in vec4 vs_gs_color[];
out vec4 gs_fs_color;
out vec2 gs_fs_texcoord;

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


    vec3 rotAxis = vs_gs_rot_axis[0];
    float theta = vs_gs_theta[0];
    vec3 origin = vec3(0.0,0.0,0.0);



	float nn = 20.0;
    int n = 20;
    
    for (int i = 0;i < n;++i)
    {
        mat4 rotateMatrix = getRotateMatrix(origin, rotAxis, (theta -i*10)*0.004 );//theta- i* 0.01
        gl_Position = gl_ModelViewProjectionMatrix * rotateMatrix * (gl_in[0].gl_Position + sin((theta*0.0001 -i*0.001) + rotAxis.x) * vec4(-gl_in[0].gl_Position.xyz,0.0));
        gs_fs_texcoord = vec2(0.001+ i / (nn+0.1),0.5);
        EmitVertex();
        gl_Position = gl_ModelViewProjectionMatrix * rotateMatrix * (gl_in[2].gl_Position  + sin((theta*0.0001 -i*0.001) + rotAxis.x) * vec4(-gl_in[2].gl_Position.xyz,0.0));
        gs_fs_texcoord = vec2(0.001+ i / (nn+0.1),0.5);
        EmitVertex();
    }
    EndPrimitive();
}