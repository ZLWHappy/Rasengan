#version 330 compatibility

uniform sampler2D chakraTexture;
 

in vec4 vs_fs_color;
in vec4 gs_fs_color;
in vec2 gs_fs_texcoord;
void main()
{
	//gl_FragColor = vec4(18.0f/255.0f, 150.0f/255.0f, 219.0f/255.0f,1.0);
	//gl_FragColor = vs_fs_color;
	//gl_FragColor = gs_fs_color;
	gl_FragColor = texture(chakraTexture, gs_fs_texcoord);
	
}