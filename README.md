### 2. **Integrate the Draco Decoder**

To integrate the **Draco decoder**, do the following:

* Include the Draco decoder sources from the [Blender repo](https://github.com/KhronosGroup/glTF-Blender-IO/tree/main/addons/io_scene_gltf2/blender/exp/draco) or the [official repo](https://github.com/google/draco).
* Compile Draco as part of your extension or as a static library.
* When parsing GLTF, look for the `KHR_draco_mesh_compression` extension in the mesh primitives:

```json
"extensions": {
  "KHR_draco_mesh_compression": {
    "bufferView": 2,
    "attributes": {
      "POSITION": 0,
      "NORMAL": 1
    }
  }
}
```

* Decode the bufferView using Draco.
* Reconstruct the mesh data and feed it into `GLTFDocument`.

### 4. **Decoding Workflow**

Basic Draco decoding flow:

```cpp
draco::Decoder decoder;
draco::DecoderBuffer buffer;

buffer.Init(buffer_data_ptr, buffer_data_size);

auto maybe_geometry = decoder.DecodeMeshFromBuffer(&buffer);
if (!maybe_geometry.ok()) {
    // Handle error
}
const draco::Mesh &mesh = maybe_geometry.value();
```

Then extract attributes and create a `MeshData` or feed into `GLTFDocument`.

_____________________________________________________________________________________________________
✅ How to attach the decoded mesh?

After decoding:

Ref<Mesh> decoded_mesh = decode_draco_mesh(draco_data);


You attach it to the GLTFNode’s mesh reference:

Ref<GLTFMesh> new_mesh;
new_mesh.instantiate(); // Create a new GLTFMesh resource
new_mesh->set_mesh(decoded_mesh); // Set the actual decoded Godot Mesh

p_gltf_node->set_mesh(new_mesh);

Or more commonly:

If you already know the GLTFMesh is in the GLTF state (by mesh index), you could:

int mesh_index = p_gltf_node->mesh; // index into GLTFState’s mesh array
Ref<GLTFMesh> gltf_mesh = p_state->get_meshes()[mesh_index];
gltf_mesh->set_mesh(decoded_mesh); // Plug in the decoded mesh


🔁 Later, when the GLTFDocument finishes importing:

It builds the final Godot scene from the GLTFNode tree.