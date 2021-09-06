// Fragment shader: : Phong shading
// ================
#version 450 core

in vec3 N;
in vec3 L;
in vec3 E;
in vec2 _TexCoord;

// Ouput data
out vec4 FragColor;
uniform sampler2D textureBuffer;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
}; 

struct PointLight{
	vec3 position;
	vec3 color;
	float power;
 };

uniform Material material;
uniform PointLight light;

//i passaggi per lo shader phong con texture sono gli stessi visti in LAB_03
//ma la componente diffuse dell'equazione è data dalla texture in quel fragment
void main()
{
	// ambient
	vec3 ambient = material.ambient;    

    // diffuse 
    vec3 norm = normalize(N);
	vec3 lightDir = normalize(L);
    float diff = max(dot(norm, lightDir), 0.0);
	//al posto di material.diffuse, passo il colore della texture (canale rgb)
    vec3 diffuse = light.power * light.color * diff * texture(textureBuffer,_TexCoord).rgb;
    
    // specular
    vec3 viewDir = normalize(E);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular =  light.power * light.color * spec * material.specular;  
        
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}