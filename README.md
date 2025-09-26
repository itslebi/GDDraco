# GDDraco
#### Godot GDExtension: Draco-Compressed GLTF/GLB Import Support

This GDExtension adds support to the Godot Engine for importing `.gltf` and `.glb` files with **Draco-compressed meshes** via the [`KHR_draco_mesh_compression`](https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_draco_mesh_compression/README.md) extension.

This extension uses the Draco Decoder found in Blender's [Draco SDK Wrapper](https://github.com/blender/blender/tree/2d4984493155e3f23eed34dcae8e7afe08aefbde).

---

## Index

- [Features](#-features)
- [How to Use](#how-to-use)
  - [Prerequisites](#prerequisites)
  - [Download the Latest Release](#1-download-the-latest-release)
- [Developing Build](#developing-build)
  - [Prerequisites](#prerequisites-1)
  - [1. Clone the Repository](#1-clone-the-repository)
  - [2. Building](#2-building)
  - [3. Testing](#3-testing)
- [License](#license)
- [Credits](#credits)

---

## Features

- Full support for loading Draco-compressed geometry in glTF 2.0 files.
- Seamless integration with Godot's existing GLTF/GLB import pipeline.
- Built as a GDExtension — no need to recompile the engine.
- Cross-platform support (depending on how you build the Draco library).

---

## How to Use

### Prerequisites

- **[Godot 4.5](https://godotengine.org/)**  
  _⚠️ This extension was tested with **Godot 4.5**. It may work with other 4.x versions, but compatibility is not guaranteed._

### 1. Download the Latest Release
....

---

## Developing Build

### Prerequisites

- **[Godot 4.5](https://godotengine.org/)**  
  _⚠️ This extension was tested with **Godot 4.5**. It may work with other 4.x versions, but compatibility is not guaranteed._
- C++ build environment (GCC/Clang/MSVC)
- [Python](https://www.python.org/)
- [SCons](https://scons.org/) (used for building)

### 1. Clone the Repository

```bash
git clone https://github.com/itslebi/GDDraco
cd GDDraco
````

### 2. Building

Run the following command inside the main `GDDraco` folder:

```bash
scons
```

> ⚙️ This will build the GDExtension and output the compiled binary (`.dll` or `.so`) in the ```bin/``` directory.

### 3. Testing

Use the demo project in the `demo/` folder:

* Open the project with Godot
* Reimport the existing `.glb` file with Draco compression
* Or try importing your own files

---

## License

**GDDraco** is open source and licensed under the **GPL**.
You are free to use, modify, and distribute this software as long as the original license and copyright notice are included.

Please note:

* [Google Draco](https://github.com/google/draco) is licensed under the **Apache 2.0 License**
* [Blender](https://www.blender.org/) source code used for the wrapper is licensed under **GPL**

Make sure to verify compatibility based on how you distribute or bundle binaries.

---

## Credits
**This extension was made by me alone but it would not be possible without the below open source projects!**
* [Godot Engine](https://godotengine.org/)
* [Google Draco](https://github.com/google/draco)
* [Blender Foundation](https://www.blender.org/) — for the Draco wrapper code
* [Khronos Group](https://www.khronos.org/) — for the glTF format and specifications

---

> © 2025 [@itslebi](https://github.com/itslebi) — Contributions welcome!
