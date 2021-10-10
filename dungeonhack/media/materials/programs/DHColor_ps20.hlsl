sampler RT: register(s0);
//Dungeon Hack pseudo HDR effect, tone mapping was based on Sinbad's HDR example compositor

static const float MIDDLE_GREY = 0.72f;
static const float FUDGE = 0.001f;
static const float L_WHITE = 1.5f;
static const float LUM = 0.55f;

float4 main(float2 texCoord: TEXCOORD0) : COLOR {

   float4 col = tex2D(RT, texCoord);

   col.rgb *= MIDDLE_GREY / (FUDGE + LUM);
   col.rgb *= (1.0f + col / L_WHITE);
   col.rgb /= (1.5f + col);

   col.rgb *= (col.rgb);

  return col;

}




