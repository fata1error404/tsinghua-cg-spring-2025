<!-- markdownlint-disable MD033 MD001 MD045 -->

# Terrain Engine ⛰️

### Project overview

This project implements a terrain engine, compiled into a standalone executable. Basic goal is rendering a 3D scene that contains a skybox, water, and terrain, with an ability to fly around. Then, advanced features, such as lighting, shadows, weather and dynamic waves, add more realism into it.

The engine is written in C++ and uses __OpenGL__ 3.3 as its rendering backend (core profile, enabling full control over the graphics rendering pipeline), with __GLSL__ for programmable shaders.

### Result

<img src="aux_docs/result.png" width="80%" height="45%">

<br />

Terrain entities:

- Camera – system that emulates first-person view camera (fly around + look around + zoom)
- Skybox – large cube that encompasses the entire scene and contains 6 images of a surrounding environment
- Water – dynamic surface generated as 3D Gerstner waves
- Terrain – 3D mesh generated from a height map
- Lighting – Phong lighting model (ambient + diffuse + specular lighting) and shadows
- Weather – particle emitters for fog and rain

<br />

Folders structure:

- [`aux_docs`](./aux_docs) – auxiliary project files
- [`data`](./data) – texture images
- [`shaders`](./shaders) – GLSL shaders source code

<br />

### Manual

Install dependencies  
`sudo apt-get install mesa-utils mesa-common-dev libglu1-mesa-dev freeglut3-dev`
 –  core OpenGL utilities (libraries for rendering, handling windowing and output)  
`sudo apt-get install libglew-dev libglfw3-dev libglm-dev` – advanced development libraries (GLEW, GLFW, GLM)

Compile and launch  
`g++ main.cpp -o app -lglfw -lglad`  
`./app`

Keyboard controls:  
__W A S D__ – camera movement  
__- +__ – wave height control  
__N__ – enable / disable weather  
__L__ – enable / disable lighting  
__M__ – show / hide light cube  
__F__ – fullscreen mode  
__Escape__ – exit
