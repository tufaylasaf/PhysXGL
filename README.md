
# Custom Graphics Engine

## Overview
This project is a custom real-time 3D rendering engine built using C++ and OpenGL. The engine supports a variety of lighting models, shading techniques, and a flexible scene management system. Designed for flexibility and expandability, it integrates a powerful UI using ImGui for real-time scene manipulation and debugging.

## Features

### Lighting Models
- **Directional Light**: Simulates sunlight with adjustable intensity and direction.
- **Point Light**: Omnidirectional lighting that attenuates over distance.
- **Spotlight**: Directional lighting with adjustable cutoff angles for creating cone-shaped beams.
- **Physically Based Rendering (PBR)**: Uses realistic material properties like albedo, metallic, roughness, and ambient occlusion for rendering.
- **Shadow Mapping**: Dynamic shadow rendering for all light types (Directional, Point, and Spot).

### Scene Management
- **3D Model Loading**: Support for loading 3D models (OBJ, FBX) with textures.
- **Material Editing**: Real-time control over material properties (ambient, diffuse, specular, shininess).
- **Transform Controls**: Adjust translation, rotation, and scale of objects in the scene.
- **UI Integration with ImGui**: Easy-to-use, customizable UI for manipulating lights, materials, and objects in real time.

### Shader Effects
- **Custom Shader System**: Allows easy creation, loading, and management of GLSL shaders.
- **Blinn-Phong and PBR Shading**: Choose between classic lighting models and more advanced physically-based models.
- **Multiple Light Sources**: Handles multiple point lights with efficient shading and shadowing.

### Additional Features
- **Camera System**: Move and rotate the camera in 3D space with WASD and mouse controls.
- **Optimized Rendering Pipeline**: Efficient handling of multiple lights and complex shaders.
- **Extensible Framework**: Easily add new lights, shaders, and models to the engine.
