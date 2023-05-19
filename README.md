# Graphic Processing Project
This is a repository containing my project for the Graphic Processing laboratory from my university curriculum. The aim of this assignment is to develop a "3D game". The "game" works more like a 3D model viewer, where you can move the camera wherever you want and modify properties of the world.

## Setting up
The project is compiled using Microsoft Visual Studio 2022. The necessary GLFW and GLEW libraries are already built and included in the project, so the project should be compiled normally with Visual Studio without any problem.

## Features
The user can navigate the scene using the mouse and keyboard and switch between four viewing modes using keyboard keys: solid, wireframe, point, and depth map view.<br />
The scene also has a distant fog and multiple sources of light. The sun is one of them, which produces directional light over the city and the objects, and every traffic light is a red point light source.<br />
Along with moving the camera, the user can rotate the character from the scene. The user is also able to switch to a “auto camera” which goes through the scene. <br />

## Controls
The user can interact with the program in the following ways: 
<ul>
  <li>Mouse: rotate the camera.</li>
  <li>WASD: move the camera forward, left, backwards and right,</li>
  <li>Q and E: move the camera up and down,</li>
  <li>J and L: move “the sun”,</li>
  <li>M and N: rotate the character,</li>
  <li>Z: switch to solid view,</li>
  <li>X: switch to wireframe view,</li>
  <li>C: switch to point view,</li>
  <li>V: switch to depth map view,</li>
  <li>Y: toggle on and off automatic camera.</li>
</ul>