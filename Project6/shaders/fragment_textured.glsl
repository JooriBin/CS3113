
uniform sampler2D diffuse;
uniform int playerIsDead;

varying vec2 texCoordVar;

void main() {
    
    if(playerIsDead == 1){
        vec4 colour = texture2D(diffuse, texCoordVar);
        float c = (colour.r + colour.g + colour.b) / 3.0;
        gl_FragColor = vec4(c, c, c, colour.a);
    }
    else{
        gl_FragColor = texture2D(diffuse, texCoordVar);

    }
}
