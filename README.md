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
## ✅ Modify Your Header and Class Declaration

You need to:

1. Include the right Godot headers.
2. Inherit from `GLTFDocumentExtension`.
3. Register your extension with the GLTF system.
_________________________________________________________________
### 2. `_parse_node_extensions()`

* **Purpose:** Process the JSON data for the extension on each node.
* **What to do:** Decode the Draco data or perform whatever processing your extension requires.
* **When called:** For each node that *has* extensions, right after `_get_supported_extensions()` confirms your extension applies.

---

### Minimal example:

```cpp
Array GDDraco::_get_supported_extensions() const override {
    Array extensions;
    extensions.append("KHR_draco_mesh_compression");
    return extensions;
}

Error GDDraco::_parse_node_extensions(
    const Ref<GLTFState> &state,
    const Ref<GLTFNode> &gltf_node,
    const Dictionary &extensions
) override {
    if (!extensions.has("KHR_draco_mesh_compression")) {
        return ERR_SKIP;
    }

    UtilityFunctions::print("Draco extension found!");

    // Your Draco decoding logic here

    return OK;
}
```

---

### Bonus:

You might also want to override:

* `_import_post_parse()` — for any final steps after all nodes are parsed.
* `_generate_scene_node()` — to customize how the decoded data creates Godot nodes/meshes.

---

### TL;DR:

**Yes, you need to override `_get_supported_extensions()` to declare support, and `_parse_node_extensions()` to process your extension on each node.**

---

If you want, I can help you write full skeleton code for these functions and how to register your extension properly!

__________________________________
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

________________

## 🔍 What is `_import_preflight()` for?

`_import_preflight()` allows an extension (like your `GDDraco` handler) to:

1. **Check if the GLTF file uses the extension** (e.g., `KHR_draco_mesh_compression`).
2. **Prepare internal state**, if needed, before the rest of the GLTF is parsed.
3. **Communicate capabilities or requirements** to the import process.

This method is especially useful when:

* You need to **set up flags** that influence how later parsing happens.
* You need to **allocate resources early**, or pre-validate data.
* You want to **record that an extension is used** even if it's optional.

---

## 🧠 Why do anything here for `KHR_draco_mesh_compression`?

The GLTF spec defines `KHR_draco_mesh_compression` as a **mesh-level compression extension**. If it's used in the file, you want to be ready for it.

So in this function:

```cpp
Error GDDraco::_import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) {
    if (p_extensions.has("KHR_draco_mesh_compression")) {
        // Maybe set a flag in the GLTFState custom state
    }
    return OK;
}
```

You should use it to **mark that Draco decoding will be needed later**. That way, your `_parse_node_extensions` or `_parse_mesh` functions can act accordingly.

---

## ✅ What should be done here?

### ✅ Set a custom flag in the `GLTFState` to mark Draco usage.

The `GLTFState` object has a `Dictionary` field called `custom_data` specifically for this purpose.

### Example:

```cpp
Error GDDraco::_import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) {
    if (p_extensions.has("KHR_draco_mesh_compression")) {
        Dictionary custom = p_state->get_custom_data();
        custom["use_draco"] = true;
        p_state->set_custom_data(custom);
    }
    return OK;
}
```

---

## 🧭 Why set `use_draco` in `custom_data`?

* ✅ Lets you avoid rechecking the extension everywhere later.
* ✅ Allows you to **skip Draco-related parsing** if the flag is not set.
* ✅ Keeps your parsing logic cleaner and centralized.
* ✅ Can be used in `_parse_mesh`, `_parse_node_extensions`, etc., to short-circuit logic.

---

## 🧪 How is this used later?

Later in `_parse_node_extensions`, you might do:

```cpp
Dictionary custom = p_state->get_custom_data();
if (!custom.has("use_draco") || !bool(custom["use_draco"])) {
    return ERR_SKIP;
}
```

Or if you cache buffer indices or decoder settings in `custom_data`, you can read them later without having to reparse extension declarations.

---

## 🧩 Summary

| Purpose of `_import_preflight()` | What to do                               |
| -------------------------------- | ---------------------------------------- |
| Detect if the extension is used  | Check for `"KHR_draco_mesh_compression"` |
| Prepare for decoding later       | Set a custom flag in `GLTFState`         |
| Avoid redundancy or re-checks    | Store state like `custom["use_draco"]`   |

---

## 🛠 Recommended Code to Add

```cpp
Error GDDraco::_import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) {
    if (p_extensions.has("KHR_draco_mesh_compression")) {
        Dictionary custom = p_state->get_custom_data();
        custom["use_draco"] = true;
        p_state->set_custom_data(custom);
        UtilityFunctions::print("Draco extension detected in preflight.");
    }
    return OK;
}
```