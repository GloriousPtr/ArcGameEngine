# Arc Game Engine

[![Build Status](https://github.com/MohitSethi99/ArcEngine/workflows/build/badge.svg)](https://github.com/MohitSethi99/ArcEngine/actions?workflow=build)
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=MohitSethi99_ArcGameEngine&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=MohitSethi99_ArcGameEngine)
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=MohitSethi99_ArcGameEngine&metric=bugs)](https://sonarcloud.io/summary/new_code?id=MohitSethi99_ArcGameEngine)
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=MohitSethi99_ArcGameEngine&metric=code_smells)](https://sonarcloud.io/summary/new_code?id=MohitSethi99_ArcGameEngine)
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=MohitSethi99_ArcGameEngine&metric=reliability_rating)](https://sonarcloud.io/summary/new_code?id=MohitSethi99_ArcGameEngine)

![Platform](https://img.shields.io/badge/platform-Windows%20%28Primary%29%20%7C%20Linux%20%28Experimental%29-blue?style=flat-square)
![GitHub](https://img.shields.io/github/license/MohitSethi99/ArcEngine?color=blue&style=flat-square)
![Size](https://img.shields.io/github/repo-size/MohitSethi99/ArcEngine?style=flat-square)

![ARC](https://MohitSethi99.github.io/images/ArcGameEngine/Sponza.jpg)

## About

Arc Engine is an in-development game engine written in C++.

I develop it in my spare time as a personal project, so expect frequent periods of time with no activity in this repository.

## Set up

- Clone Arc Game Engine using git. Make sure you do a ```--recursive``` clone!
```
git clone --recursive https://github.com/MohitSethi99/ArcGameEngine.git
```

### Windows
- Arc Game Engine is primarily developed in a Windows environment using Visual Studio 2022.
- Run `scripts/Win-GenProjects.bat` to generate the solution and project files.

### Linux (Experimental)
Linux support is available but experimental. Tested with Clang 18.1.3 on Ubuntu/Debian.

1. Install dependencies:
   ```bash
   # Debian/Ubuntu
   sudo apt install build-essential clang libc++-dev libstdc++-14-dev
   # Or use the provided script:
   ./scripts/InstallDependencies.sh
   ```
2. Generate Makefiles and build:
   ```bash
   ./scripts/GenerateMake.sh && make config=debug -j$(nproc)
   ```

> **Note:** Windows remains the primary development platform. Linux builds may require additional maintenance as the project evolves.

## Current Features

Right now, Arc Game Engine is very bare-bones. Its most remarkable features are:

| Feature               | Description                                                                           | Status   |
| -------               | -----------                                                                           | ------   |
| **Scene system**      | Scene-based entity management. Scene serialization with YAML.                         | Done ✔️ |
| **ECS support**       | Entity Component System approach through *entt*.                                      | Done ✔️ |
| **2D Renderer**       | 2D batch renderer supporting OpenGL.                                                  | Done ✔️ |
| **3D Renderer**       | Basic 3D renderer with lighting, IBL and PBR workflow supporting OpenGL.              | Done ✔️ |
| **ImGui support**     | Basic game editor and in-game debug UI using *dear imgui*.                            | Done ✔️ |
| **Shadow Support**    | Soft and Hard shadow support in OpenGL 3D Renderer.                                   | Done ✔️ |
| **2D Physics**        | 2D Physics using *Box2D* with Rigidbody, Box and Circle Colliders and joints.         | Done ✔️ |
| **3D Physics**        | Basic 3D Physics using *Jolt Physics* with Rigidbody, Box and Sphere Colliders.   	  | Done ✔️ |
| **Scripting**         | Basic scripting in C# using *mono*.                                                   | Done ✔️ |
| **Audio**             | Basic Sound API with spatialization through *miniaudio*.                              | Done ✔️ |

## Planned Features

| Feature                | Description                                               | Status   |
| -------                | -----------                                               | ------   |
| **C# API**             | C# API for all the components.                            | WIP 💻  |
| **Extended 2D Physics**| Extend 2D physics to support more collider types and API. | WIP 💻  |
| **Extended 3D Physics**| Extend 3D physics to support more collider types and API. | WIP 💻  |
| **Font**               | Extend the Renderer capabilities so it can draw text.     | WIP 💻  |
| **Advanced Audio**     | Advanced Sound API with effects.                          | TODO 📋 |
| **Asset manager**      | Basic asset handling API.                                 | TODO 📋 |
| **Direct3D**           | Extend the Renderer to support D3D12.                     | TODO 📋 |

## Feature Showcase

[![Arc Game Engine Showcase](https://MohitSethi99.github.io/images/ArcGameEngine/VideoThumbnail.jpg)](https://youtu.be/bUqAVP7_XAw)

## Dependencies

Arc Game Engine has the following dependencies:

  - [assimp](https://github.com/assimp/assimp) for loading meshes.
  - [box2d](https://github.com/erincatto/box2d.git) for 2D physics.
  - [dear imgui](https://github.com/ocornut/imgui) for GUI rendering.
  - [EABase](https://github.com/electronicarts/EABase) dependency for EASTL.
  - [EASTL](https://github.com/electronicarts/EASTL) replacement for stl.
  - [entt](https://github.com/skypjack/entt) for ECS management.
  - [Glad](https://glad.dav1d.de) for OpenGL rendering.
  - [GLFW](https://github.com/glfw/glfw) for OpenGL, OpenGL ES, window creation and input handling. More info at its [website](https://www.glfw.org/).
  - [glm](https://github.com/g-truc//glm) as math(s) library.
  - [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) for in editor transformation gizmos.
  - [Jolt Physics](https://github.com/jrouwe/JoltPhysics) for 3D physics.
  - [miniaudio](https://github.com/mackron/miniaudio) for audio.
  - [mono](https://github.com/mono/mono) for C# scripting.
  - [optick](https://github.com/bombomby/optick) for profiling.
  - [spdlog](https://github.com/gabime/spdlog) for logging.
  - [stb_image.h](https://github.com/nothings/stb) for loading textures.
  - [yaml-cpp](https://github.com/jbeder/yaml-cpp) for scene serialization in YAML.

Projects are generated with [Premake 5](https://github.com/premake/premake-core/releases).

## Special Mentions

Huge shout out to:

  - Yan Chernikov's [video series](https://www.youtube.com/playlist?list=PLlrATfBNZ98dC-V-N3m0Go4deliWHPFwT). Original code at [TheCherno/Hazel](https://github.com/TheCherno/Hazel).
  - LearnOpenGL [OpenGL tutorials](https://learnopengl.com).
  - Jason Gregory's [Game Engine Architecture](https://www.gameenginebook.com) book.
  
