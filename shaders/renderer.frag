// renderer fragment shader
#version 330 core

in vec2 uv;
out vec4 fragColor;

// gl_Position이 필요하면, gl_FragCoord를 사용한다.
uniform sampler2D gTex;

void main()
{
    fragColor = texture(gTex, uv);
}
