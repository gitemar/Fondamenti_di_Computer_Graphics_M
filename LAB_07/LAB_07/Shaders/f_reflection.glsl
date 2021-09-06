out vec4 FragColor;
in vec3 Normal;
in vec3 Position;

uniform vec3 camera_position;
//tipo specifico per gestire texture cubiche 
uniform samplerCube cubemap;

void main(){
	
	//per calcolare il colore dell'oggetto vado a prendere il colore
	//della faccia/texture del cubo nell'intersezione con il raggio riflesso
	vec3 E = normalize(Position - camera_position);
	vec3 R = reflect(E, normalize(Normal));
	
	FragColor = texture(cubemap, R);
}