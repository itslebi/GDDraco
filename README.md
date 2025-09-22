## ✅ Current Setup Summary

* **Goal**: Create a Godot GDNative/GDExtension to support loading Draco-compressed GLTF files.
* **Environment**: Working GDExtension setup already tested with a basic node.
* **Resources**:

  * You have the Draco decoder (from Blender or elsewhere).
  * You're considering following the Blender GLTF addon as a guide.
  * You're considering subclassing `GLTFDocument` in Godot.

---

## 🧠 Suggested Architecture / Next Steps

Here’s a possible plan forward:

---

### 1. **Subclass `GLTFDocument` or Wrap It?**

Instead of subclassing directly (since `GLTFDocument` is not designed to be extended virtually from GDExtensions), consider this:

* Use composition or a wrapper pattern.
* Register your own loader that **uses `GLTFDocument`** internally but pre-processes the file to decode Draco-compressed data first.
* Hook into the GLTF import process via `GLTFDocumentExtension` if possible.

If subclassing `GLTFDocument` **isn't viable** due to internal Godot constraints (which may be the case unless you expose Godot internals via C++ module instead of GDExtension), consider:

```cpp
Ref<GLTFDocument> doc;
doc.instantiate();
doc->set_extensions(...); // Add your Draco decoder logic here
doc->parse_gltf(path);
```

---

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

---

### 3. **Use `GLTFDocumentExtension` Interface (Optional)**

Godot 4 introduced **`GLTFDocumentExtension`**, which is an API that allows custom extensions to the GLTF pipeline.

If you're using Godot 4.2+ and writing your extension in C++, implement this interface:

```cpp
class MyDracoImporter : public GLTFDocumentExtension {
    GDCLASS(MyDracoImporter, GLTFDocumentExtension);

public:
    virtual Error parse_node_extensions(GLTFDocument *document, Ref<GLTFState> state, Dictionary &json, int node_index) override {
        // Your Draco handling logic
    }

    virtual Error parse_mesh_primitive_extensions(GLTFDocument *document, Ref<GLTFState> state, Dictionary &json, Ref<GLTFMesh> mesh, int primitive_index) override {
        // Draco mesh decoding logic
    }
};
```

Then register it with:

```cpp
GLTFDocument::register_gltf_document_extension(memnew(MyDracoImporter));
```

---

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

---

### 5. **Testing**

* Use a GLTF file with **Draco compression** (you can export one from Blender with Draco compression enabled).
* Create unit tests to compare decoded mesh vs. non-compressed version.
* Use `ResourceLoader::load()` to test importing through your GDExtension.

---

### 6. **Notes on Blender’s GLTF Exporter**

* The [Blender exporter](https://github.com/KhronosGroup/glTF-Blender-IO) handles Draco compression via its own exporter logic.
* Their Python code shows how attributes are mapped and encoded.
* While it's Python, the logic flow can still help map between GLTF mesh JSON structure and actual data buffer.

---

## 🛠️ Optional Enhancements

* Add support for other extensions while you're in there (e.g., `KHR_materials_unlit`).
* Use `MeshDataTool` in Godot to manipulate or inspect the decoded mesh.

---

## 🧩 Sample Project Structure

```
my_draco_importer/
├── draco/                # Draco source code
├── src/
│   ├── register_types.cpp
│   ├── my_draco_importer.cpp
│   ├── my_draco_importer.h
├── godot/
│   ├── extension.gdextension
│   ├── mesh_viewer_node.gd
```

---

## 💬 Final Thoughts

You're well-prepared for this, and using the `GLTFDocumentExtension` path is likely the cleanest way to go with GDExtensions. If you hit any Godot-side limitations (e.g., needing to access internals not exposed to extensions), you might need to build this as a **custom module** instead of GDExtension—but try to avoid that if possible.

Let me know if you'd like a sample C++ Draco decoder stub or help wiring the extension into Godot!

_____________________________________________________________________________________________________
## ✅ Modify Your Header and Class Declaration

You need to:

1. Include the right Godot headers.
2. Inherit from `GLTFDocumentExtension`.
3. Register your extension with the GLTF system.

---

### ✅ Step 1: Include Required Headers

```cpp
#include "modules/gltf/gltf_document_extension.h"
#include "modules/gltf/gltf_state.h"
#include "modules/gltf/gltf_mesh.h"
#include "modules/gltf/gltf_mesh_primitive.h"
#include "core/object/class_db.h"
```

> Adjust include paths if your module is outside `modules/`.

---

### ✅ Step 2: Update Your Class Declaration

```cpp
#pragma once

#include "modules/gltf/gltf_document_extension.h"

class GDDraco : public GLTFDocumentExtension {
    GDCLASS(GDDraco, GLTFDocumentExtension);

public:
    GDDraco();
    ~GDDraco();

    // Called during GLTF parsing to process mesh primitive extensions
    virtual Error parse_mesh_primitive_extensions(
        GLTFState &state,
        const Dictionary &extensions,
        Ref<GLTFMesh> mesh,
        Ref<GLTFMeshPrimitive> primitive
    ) override;

    static void _bind_methods();
};
```

---

### ✅ Step 3: Implement the Methods

```cpp
#include "GDDraco.hpp"

GDDraco::GDDraco() {}
GDDraco::~GDDraco() {}

void GDDraco::_bind_methods() {
    // No binding needed unless exposing to GDScript.
}

Error GDDraco::parse_mesh_primitive_extensions(
    GLTFState &state,
    const Dictionary &extensions,
    Ref<GLTFMesh> mesh,
    Ref<GLTFMeshPrimitive> primitive
) {
    if (!extensions.has("KHR_draco_mesh_compression")) {
        return ERR_SKIP;
    }

    Dictionary draco_ext = extensions["KHR_draco_mesh_compression"];

    // TODO: Get bufferView, decompress it using Draco,
    // and write to `primitive`.

    // For now, just print debug info
    print_line("Draco compression found!");

    return OK;
}
```

---

### ✅ Step 4: Register the Extension in Your Module

In your module's `register_types.cpp`:

```cpp
#include "GDDraco.hpp"
#include "modules/gltf/gltf_document.h"

void register_gddraco_types() {
    GLTFDocument::get_singleton()->register_gltf_document_extension(memnew(GDDraco));
}

void unregister_gddraco_types() {
    // nothing to unregister
}
```

Make sure this is called by your module's init functions.

---

### ✅ Step 5: Build Your Module

Your SConstruct should compile the Draco decoder and your `GDDraco.cpp`.

---

## ✅ Next Steps

Now that your extension is properly registered and set up:

* You can detect Draco-compressed primitives.
* Next, integrate the **Draco C++ decoder** in your module and use it inside `parse_mesh_primitive_extensions()`.

If you want, I can help you write the actual Draco decoding logic inside that function.

Let me know!

_________________________________________________________________
Exactly! To properly support a GLTF extension like `KHR_draco_mesh_compression` in Godot’s GLTF import pipeline, you typically need to override **at least these two methods** in your `GLTFDocumentExtension` subclass:

---

### 1. `_get_supported_extensions()`

* **Purpose:** Tell Godot *which* GLTF extensions your code handles.
* **What to do:** Return an array or list of extension names your extension supports, e.g., `["KHR_draco_mesh_compression"]`.
* **When called:** Before import starts, so Godot knows when to call your extension.

---

### 2. `_parse_node_extensions()`

* **Purpose:** Process the JSON data for the extension on each node.
* **What to do:** Decode the Draco data or perform whatever processing your extension requires.
* **When called:** For each node that *has* extensions, right after `_get_supported_extensions()` confirms your extension applies.

---

### Why both?

* `_get_supported_extensions()` **advertises** your extension support.
* `_parse_node_extensions()` **does the actual work** on each relevant node.

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