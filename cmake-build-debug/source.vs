#version 330 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 fragColor;
layout (location = 2) in vec3 normal;
uniform int zBufferRenderMode;
uniform mat4 matrix;
uniform float nearClippingPlane;
uniform float farClippingPlane;
uniform int shadingMode;
uniform float ambientLightIntensity;
uniform float lightIntensity;
uniform float phongExponent;
uniform vec3 lightVec;
uniform vec3 specularColor;
out vec4 vFragColor;
out vec3 vFragNormal;
void main()
{
   gl_Position = matrix * vec4(aPos.x, aPos.y, aPos.z, aPos.w);
   vFragNormal = normalize(vec3(matrix * vec4(normal, 0)));

   // None
   if (zBufferRenderMode == 0) {
       // None or Phong
       if (shadingMode == 0 || shadingMode == 3) {
           vFragColor = fragColor;
       }
       // Flat or Gouraud
       else if (shadingMode == 1 || shadingMode == 2) {
           // Shading
           vec3 ambientLight = ambientLightIntensity * vec3(fragColor);
           vec3 diffuseLight = lightIntensity * max(0, dot(vFragNormal, -1 * lightVec)) * vec3(fragColor);
           vec3 eyeVec = vec3(0, 0, -1);
           vec3 h = normalize(eyeVec + -1 * lightVec);
           vec3 specularLight = lightIntensity * max(0, pow(dot(vFragNormal, h), phongExponent)) * specularColor;
           vec4 newColor = vec4(ambientLight + diffuseLight + specularLight, vFragColor[3]);

           newColor[0] = min(1, newColor[0]);
           newColor[1] = min(1, newColor[1]);
           newColor[2] = min(1, newColor[2]);

           vFragColor = newColor;
       }
       else {
           vFragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
       }
   }
   // ZMode
   else if (zBufferRenderMode == 1) {
       float zVal = -1 * gl_Position.w;

       // Normalize to [0, 1]
       float nMinus = -1 * nearClippingPlane;
       float fMinus = -1 * farClippingPlane;
       zVal = (zVal - nMinus) / (fMinus - nMinus);

       vFragColor = vec4(zVal, zVal, zVal, 1.0f);
   }
   // ZTildeMode
   else if (zBufferRenderMode == 2) {
       float zTildeVal = gl_Position.z;

       if (gl_Position.w == 0) {
          zTildeVal = 0;
       }
       else {
          // Normalize to [0, 1]
          zTildeVal = (zTildeVal + gl_Position.w) / (2 * gl_Position.w);
       }

       vFragColor = vec4(zTildeVal, zTildeVal, zTildeVal, 1.0f);
   }
   // ZPrimeMode
   else if (zBufferRenderMode == 3) {
       float zPrimeVal;
       if (gl_Position.w == 0) {
           zPrimeVal = 0;
       }
       else {
           zPrimeVal = gl_Position.z / gl_Position.w;
       }

       // Normalize from [-1, 1] to [0, 1]
       zPrimeVal = (zPrimeVal + 1) / 2.0;

       vFragColor = vec4(zPrimeVal, zPrimeVal, zPrimeVal, 1.0f);
   }
   else {
       vFragColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
   }
}
