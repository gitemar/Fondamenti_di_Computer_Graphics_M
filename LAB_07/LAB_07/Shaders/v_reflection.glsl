
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

void main(){

	//ho bisogno di lavorare con vettori 3D
	//per lavorare nel WCS devo lavorare solo con M, non con anche V e Position
	//per lavorare con M, devo fare inversa e poi trasposta
	Normal = mat3(transpose(inverse(M))) * aNormal;
	Position = vec3(M*vec4(aPos,1.0));
	
	gl_Position = P * V * M * vec4(aPos,1.0);
}