#version 330 core
in vec4 vFragColor;
in vec3 vFragNormal;
out vec4 FragColor;
uniform int zBufferRenderMode;
uniform int shadingMode;
uniform float ambientLightIntensity;
uniform float lightIntensity;
uniform float phongExponent;
uniform vec3 lightVec;
uniform vec3 specularColor;
void main()
{
   // ZMode or ZTildeMode or ZPrimeMode or None or Flat or Gouraud
   if (zBufferRenderMode == 1 || zBufferRenderMode == 2 || zBufferRenderMode == 3 || shadingMode == 0 || shadingMode == 1 || shadingMode == 2) {
       FragColor = vFragColor;
   }
   // Phong
   else if (shadingMode == 3) {
       // Shading
       vec3 ambientLight = ambientLightIntensity * vec3(vFragColor);
       vec3 diffuseLight = lightIntensity * max(0, dot(vFragNormal, -1 * lightVec)) * vec3(vFragColor);
       vec3 eyeVec = vec3(0, 0, -1);
       vec3 h = normalize(eyeVec + -1 * lightVec);
       vec3 specularLight = lightIntensity * max(0, pow(dot(vFragNormal, h), phongExponent)) * specularColor;
       vec4 newColor = vec4(ambientLight + diffuseLight + specularLight, vFragColor[3]);

       newColor[0] = min(1, newColor[0]);
       newColor[1] = min(1, newColor[1]);
       newColor[2] = min(1, newColor[2]);

       FragColor = newColor;
   }
   else {
       FragColor = vec4(0.0f, 1.0f, 0.0f, 0.0f);
   }
}
