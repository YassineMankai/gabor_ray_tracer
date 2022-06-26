The code is based on course lab resources provided by: Tamy Boubekeur (tamy.boubekeur@telecom-paris.fr)
I also adapted the code base provided with the  “Procedural Noise using Sparse Gabor Convolution” article

### Building

Add the External folder used for the lab sessions 


This is a standard CMake project. Building it consists in running:

```
cd <path-to-MyRenderer-directory>
mkdir Build
cd Build
cmake ..
cd ..
cmake --build Build
```

The resuling MyRenderer executable is automatically copied to the root MyRenderer directory, so that resources (shaders, meshes) can be loaded easily. By default, the program is compile in Debug mode. For a high performance Release binary, just us:

```
cmake --build Build --config Release
```

### Info:
I implemented three types of noise as described in the article: 2d texture based, (setup-free) surface noise, solid noise. The las two are implemented only for the raytracer renderer.

There are three scenes to explore (three different executables). I did not manage to implement any extra interactive feature (except for moving the camera, firing a raytracing call and activating and deactivation the aabb). For the moment, the parameters for the noise functions are hard coded. 

Scene1:
    - uniform roghness
    - uniform metalicness
    - albedo values sampled on 2D textures generated using the 2D gabor noise (some anisotropic, others isotropic).


Scene2: (should activate raytracing to see the results)
    - upper row (isotropic noise) vs lower row (anisotropic noise)
    - left column (texture noise with spherical uvs), center column (surface noise), right column (solid noise)

Scene3: 
    - Left: albedo = noise // uniform roughness and metalicness 
    - center: roughness = noise  // uniform albedo and metalicness  
    - right: metalicness = noise   // uniform roughness and albedo 

