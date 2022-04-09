# LDtkViewer

A lightweight standalone application that lets you open and navigate LDtk projects.

Supports the latest LDtk format with all the new features.

[Try it out !](https://madour.github.io/projects/LDtkViewer)

### Platforms

![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)
![Mac OS](https://img.shields.io/badge/mac%20os-000000?style=for-the-badge&logo=macos&logoColor=F0F0F0)
![Linux](https://img.shields.io/badge/Linux-bbbbbb?style=for-the-badge&logo=linux&logoColor=black)
![HTML5](https://img.shields.io/badge/html5-%23E34F26.svg?style=for-the-badge&logo=html5&logoColor=white)

### Build

The project has 2 dependencies ([sogl](https://github.com/Madour/sogl) and [LDtkLoader](https://github.com/Madour/LDtkLoader)),
but don't worry, CMake will take care of everything.

Simply run:

```
mkdir build && cd build
cmake ..
cmake --build .
```

To build for the web, install [emscripten](https://emscripten.org/docs/getting_started/downloads.html) and run
`emcmake cmake ..` instead.

### Gallery


![screenshot1](https://user-images.githubusercontent.com/11854124/161399633-6fa3a174-f943-417d-8377-471ddb666b88.png)

![screenshot3](https://user-images.githubusercontent.com/11854124/161399636-07a558aa-cfc5-46b9-862d-cae45b36c262.png)

![screenshot4](https://user-images.githubusercontent.com/11854124/161399640-6001b5cd-9f1e-45c2-b2a2-fd735f4e1f12.png)

![screenshot2](https://user-images.githubusercontent.com/11854124/161399635-df3f59ad-9dbd-4a97-8788-416c8db61107.png)
