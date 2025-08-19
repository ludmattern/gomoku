uniform sampler2D texture;
uniform vec2 uCenter;   // en pixels (fenêtre)
uniform float uRadius;  // rayon en pixels

void main()
{
    vec2 uv = gl_TexCoord[0].xy;
    vec4 col = texture2D(texture, uv);

    float d = distance(gl_FragCoord.xy, uCenter);
    float mask = step(d, uRadius); // 1 si d <= rayon, sinon 0

    gl_FragColor = vec4(col.rgb, col.a * mask);
} 