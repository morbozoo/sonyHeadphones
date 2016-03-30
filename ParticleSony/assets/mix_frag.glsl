#version 130

uniform sampler2D texScene;
uniform sampler2D texLogo;

uniform vec4 colorLogo;

uniform float alphaMix;

vec4 logoAlpha;

void main()
{
	vec4 scene = texture(texScene,  gl_TexCoord[0].st);
	vec4 logo  = texture(texLogo,   gl_TexCoord[0].st);

	if(scene.g >= 0.8 && scene.b >= 0.8 && logo.r <= 0.2 && logo.g < 0.2){
		logoAlpha.r = colorLogo.r;
		logoAlpha.g = colorLogo.g;
		logoAlpha.b = colorLogo.b;
		logoAlpha.a = 1.0;
	}else{
		logoAlpha = scene;
	}
	

	//mix(scene, logoAlpha, 1.0);

	gl_FragColor =   logoAlpha;

}