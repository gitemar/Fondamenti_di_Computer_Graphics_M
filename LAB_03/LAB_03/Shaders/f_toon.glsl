#version 450 core

in vec3 N;
in vec3 L;
in vec3 E;

// Ouput data
out vec4 FragColor;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

uniform Material material;

void main() //fragment shader
{
	//define four colors for shading, from lighter (1) to darker(4) that depends on the value passed from diffuse
	//colors 1 and 2 will be lighter that diffuse, colors 3 and 4 will be darker
	//color 1
	float lighterr = material.diffuse.r + 80.0*((1.0-material.diffuse.r)/100.0);
	float lighterg = material.diffuse.g + 80.0*((1.0-material.diffuse.g)/100.0);
	float lighterb = material.diffuse.b + 80.0*((1.0-material.diffuse.b)/100.0);
	
	//color 2
	float lightr = material.diffuse.r + ((1.0-material.diffuse.r)/2.0);
	float lightg = material.diffuse.g + ((1.0-material.diffuse.g)/2.0);
	float lightb = material.diffuse.b + ((1.0-material.diffuse.b)/2.0);
	
	//color 3
	float darkr = material.diffuse.r - (material.diffuse.r/2.0);
	float darkg = material.diffuse.g - (material.diffuse.g/2.0);
	float darkb = material.diffuse.b - (material.diffuse.b/2.0);
	
	//color 4
	float darkerr = material.diffuse.r - 80.0*(material.diffuse.r/100.0);
	float darkerg = material.diffuse.g - 80.0*(material.diffuse.g/100.0);
	float darkerb = material.diffuse.b - 80.0*(material.diffuse.b/100.0);
	
	vec4 a = vec4(lighterr,lighterg,lighterb,1.0);	
	vec4 b = vec4(lightr,lightg,lightb,1.0);
	vec4 c = vec4(material.diffuse,1.0);
	vec4 d = vec4(darkr,darkg,darkb,1.0);
	vec4 e = vec4(darkerr,darkerg,darkerb,1.0);
	
	vec4 color;
	
	//depending on intensity given by L and N (output from vertex shader) 
	//we decide which color to assign to fragment
	float intensity = dot(normalize(L),normalize(N));
	if (intensity > 0.95)
		color = a;
	else if (intensity > 0.5)
		color = b;
	else if (intensity > 0.25)
		color = c;
	else if (intensity > 0.15)
		color = d;
	else
		color = e;
	float aa = dot(normalize(E), normalize(N));
	if(aa >= 0.0 && aa < 0.30)
		color = vec4(0.0, 0.0, 0.0, 1.0);
	FragColor = color;
}