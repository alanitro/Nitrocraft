# Nitrocraft

An implementation of a small Minecraft clone using C++ and OpenGL.

## Introduction
Yet another Minecraft clone in C++ and OpenGL.\
Built primarily to learn C++ programming, computer graphics, and the internal workings of Minecraft.

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
- [x] Flood fill lighting
- [x] Day/Night cycle
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
- Voxel lighting:\
<https://www.reddit.com/r/gamedev/comments/2iru8i/fast_flood_fill_lighting_in_a_blocky_voxel_game/>\
<https://www.reddit.com/r/gamedev/comments/2k7gxt/fast_flood_fill_lighting_in_a_blocky_voxel_game/>
- Voxel raycasting:\
<https://gamedev.stackexchange.com/questions/47362/cast-ray-to-select-block-in-voxel-game>
