# Nitrocraft

An implementation of a small Minecraft clone using C++ and OpenGL.

## Introduction
Yet another insignificant Minecraft clone using C++ and OpenGL.\
This project is done mainly for learning C++, computer graphics, and internal workings of Minecraft which was a mystery to me since I was a child.


## Installation
```
git clone --recursive https://github.com/alanitro/Nitrocraft.git
cd Nitrocraft
cmake -S . -B ./build
cmake --build ./build --config [Release|Debug]
```

## Features
- [x] Infinite terrain generation
- [x] Spaghetti/Cheese cave generation
- [ ] Block place/remove
- [ ] Collision detection
- [ ] Flood fill lighting
- [ ] Day/Night cycle
- [ ] Frustum culling
- [ ] Ambient occlusion
- [ ] Basic GUI
- [ ] Multithreading

## Learning Resource (Resources I came across)
- OpenGL:\
<https://learnopengl.com>\
<https://docs.gl>
- Minecraft:\
<https://www.youtube.com/watch?v=CSa5O6knuwI&ab_channel=HenrikKniberg>\
<https://www.alanzucconi.com/2022/06/05/minecraft-world-generation>\
<https://minecraft.wiki/w/World_generation>\
<https://minecraft.fandom.com/wiki/Chunk>
- Gradient noises :\
<https://adrianb.io/2014/08/09/perlinnoise.html>\
<https://muugumuugu.github.io/bOOkshelF/generative%20art/simplexnoise.pdf>