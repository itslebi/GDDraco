#include "GDDraco.hpp"

#include <cstdlib>

using namespace godot;

void GDDraco::_bind_methods() {
    // No binding needed unless exposing to GDScript.
}

GDDraco::GDDraco() {
    UtilityFunctions::print("GDDraco constructor called");
}
GDDraco::~GDDraco() {
    UtilityFunctions::print("GDDraco destructor called");
}

Error GDDraco::_import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) {
    UtilityFunctions::print("GDDraco::_import_preflight called!");

    for (int i = 0; i < p_extensions.size(); ++i) {
        String ext = p_extensions[i];
        if (ext == "KHR_draco_mesh_compression") {
            return OK; // Proceed with Draco decoding
        }
    }

    return ERR_SKIP; // Skip processing if Draco is not used
}

Error GDDraco::_parse_node_extensions(const Ref<GLTFState> &p_state, const Ref<GLTFNode> &p_gltf_node, const Dictionary &p_extensions) {
    UtilityFunctions::print("GDDraco::_parse_node_extensions called!");

    if (!p_extensions.has("KHR_draco_mesh_compression")) {
        return ERR_SKIP;
    }

    Dictionary draco_ext = p_extensions["KHR_draco_mesh_compression"];
    if (!draco_ext.has("bufferView")) {
        UtilityFunctions::printerr("KHR_draco_mesh_compression missing bufferView");
        return ERR_INVALID_PARAMETER;
    }

    int buffer_view_index = int(draco_ext["bufferView"]);
    UtilityFunctions::print("Draco bufferView index: ", buffer_view_index);

    // Get buffer views from GLTFState
    TypedArray<Ref<GLTFBufferView>> buffer_views = p_state->get_buffer_views();
    if (buffer_view_index < 0 || buffer_view_index >= buffer_views.size()) {
        UtilityFunctions::printerr("bufferView index out of range");
        return ERR_INVALID_PARAMETER;
    }

    Ref<GLTFBufferView> buffer_view = buffer_views[buffer_view_index];

    int buffer_index = buffer_view->get_buffer();
    int byte_offset = buffer_view->get_byte_offset();
    int byte_length = buffer_view->get_byte_length();

    // Get buffers from GLTFState
    TypedArray<PackedByteArray> buffers = p_state->get_buffers();
    if (buffer_index < 0 || buffer_index >= buffers.size()) {
        UtilityFunctions::printerr("buffer index out of range");
        return ERR_INVALID_PARAMETER;
    }

    PackedByteArray buffer = buffers[buffer_index];

    if (byte_offset < 0 || byte_offset + byte_length > buffer.size()) {
        UtilityFunctions::printerr("bufferView range invalid");
        return ERR_INVALID_PARAMETER;
    }

    // Extract the compressed Draco bytes from buffer
    PackedByteArray draco_data;
    draco_data.resize(byte_length);
    memcpy(draco_data.ptrw(), buffer.ptr() + byte_offset, byte_length);

    UtilityFunctions::print("Draco compressed data size: ", draco_data.size());

    // Get the Decoded Mesh from decode_draco_mesh
    Ref<Mesh> decoded_mesh = decode_draco_mesh(draco_data);
    if (decoded_mesh.is_null()) {
        UtilityFunctions::printerr("Failed to decode Draco mesh");
        return ERR_CANT_CREATE;
    }
    UtilityFunctions::print("Decoded mesh is null? ", decoded_mesh.is_null());
    UtilityFunctions::print("Surface count: ", decoded_mesh->get_surface_count());

    // TODO: attach decoded_mesh to p_gltf_node or the scene graph as needed
    int mesh_index = p_gltf_node->get_mesh();

    TypedArray<Ref<GLTFMesh>> meshes = p_state->get_meshes();

    if (mesh_index >= 0 && mesh_index < meshes.size()) {
        Ref<GLTFMesh> gltf_mesh = meshes[mesh_index];
        gltf_mesh->set_mesh(decoded_mesh);
    } else {
        UtilityFunctions::printerr("Failed to attach decoded mesh");
        return ERR_CANT_CREATE;
    }

    UtilityFunctions::print("Draco mesh decoded successfully");
    return OK;
}

Ref<GLTFObjectModelProperty> GDDraco::_import_object_model_property(const Ref<GLTFState> &p_state, const PackedStringArray &p_split_json_pointer, const TypedArray<NodePath> &p_partial_paths) {
    UtilityFunctions::print("GDDraco::_import_object_model_property called!");
    // Look for JSON pointer like: ["meshes", "0", "primitives", "0", "extensions", "KHR_draco_mesh_compression"]
    if (p_split_json_pointer.size() >= 6 &&
        p_split_json_pointer[0] == "meshes" &&
        p_split_json_pointer[2] == "primitives" &&
        p_split_json_pointer[4] == "extensions" &&
        p_split_json_pointer[5] == "KHR_draco_mesh_compression") {

        // parse mesh and primitive indices
        int mesh_index = 0;
        int prim_index = 0;
        // p_split_json_pointer[1] is a StringName or String?
        // If it's StringName, convert or do it via .c_str()
        mesh_index = int(String(p_split_json_pointer[1]).to_int());
        prim_index = int(String(p_split_json_pointer[3]).to_int());

        String key = vformat("draco_ext_mesh_%d_prim_%d", mesh_index, prim_index);

        // Create property
        Ref<GLTFObjectModelProperty> prop;
        prop.instantiate();  // ensure it's non-null
        
        prop->set_object_model_type(GLTFObjectModelProperty::GLTF_OBJECT_MODEL_TYPE_UNKNOWN);

        // If available: set JSON pointers so Godot knows which JSON this property refers to
        TypedArray<PackedStringArray> pointers_array;
        pointers_array.append(p_split_json_pointer);

        prop->set_json_pointers(pointers_array);
        // If available: map to node paths so Godot knows which node in scene this relates to
        // partial_paths may contain node path
        prop->set_node_paths(p_partial_paths);

        return prop;
    }

    // return null (empty Ref) if not interested
    return Ref<GLTFObjectModelProperty>();
}

Error GDDraco::_import_post_parse(const Ref<GLTFState> &p_state) {
    UtilityFunctions::print("GDDraco::_import_post_parse called!");

    // Get buffer views from GLTFState
    TypedArray<Ref<GLTFBufferView>> buffer_views = p_state->get_buffer_views();

    //Iterate through all of the buffers and skip the ones that are for PNGs
    for (int i = 0; i < buffer_views.size(); i++) {
        Ref<GLTFBufferView> buffer_view = buffer_views[i];

        int buffer_index = buffer_view->get_buffer();
        int byte_offset = buffer_view->get_byte_offset();
        int byte_length = buffer_view->get_byte_length();

        //Verify if buffer is valid
        PackedByteArray buffer = buffer_view->load_buffer_view_data(p_state);
        if (byte_offset < 0 || byte_offset + byte_length > buffer.size()) {
            UtilityFunctions::printerr("bufferView range invalid");
            return ERR_INVALID_PARAMETER;
        }
        if (!is_png_from_buffer_view(buffer, p_state)) {
            UtilityFunctions::print("Found a mesh!");

            // Extract the compressed Draco bytes from buffer
            PackedByteArray draco_data;
            draco_data.resize(byte_length);
            memcpy(draco_data.ptrw(), buffer.ptr() + byte_offset, byte_length);

            UtilityFunctions::print("Draco compressed data size: ", draco_data.size());

            // Get the Decoded Mesh from decode_draco_mesh
            Ref<Mesh> decoded_mesh = decode_draco_mesh(draco_data);
            if (decoded_mesh.is_null()) {
                UtilityFunctions::printerr("Failed to decode Draco mesh");
                return ERR_CANT_CREATE;
            }
            UtilityFunctions::print("Decoded mesh is null? ", decoded_mesh.is_null());
            UtilityFunctions::print("Surface count: ", decoded_mesh->get_surface_count());

            //Set the mesh to the decoded version ???
            //Maybe set the buffer to the decoded buffer!!!
        } else {
            UtilityFunctions::print("Found an image!");
        }
    }

    return OK;
}

PackedStringArray GDDraco::_get_supported_extensions() {
    UtilityFunctions::print("GDDraco::_get_supported_extensions called!");

    PackedStringArray extensions;
    extensions.append("KHR_draco_mesh_compression");
    UtilityFunctions::print(extensions);
    return extensions;
}


Ref<Mesh> GDDraco::decode_draco_mesh(const PackedByteArray &compressed_data) {
    UtilityFunctions::print("GDDraco::decode_draco_mesh called!");

    // Use ComponentType enum values
    const size_t component_type_float = 5126;       // Float
    const size_t component_type_uint = 5125;        // UnsignedInt

    Decoder* decoder = decoderCreate();
    if (!decoderDecode(decoder, (void*)compressed_data.ptr(), compressed_data.size())) {
        UtilityFunctions::printerr("Failed to decode Draco mesh");
        decoderRelease(decoder);
        return Ref<Mesh>();
    }

    uint32_t vertex_count = decoderGetVertexCount(decoder);
    uint32_t index_count = decoderGetIndexCount(decoder);

    // Assume POSITION attribute id = 0
    uint32_t position_attribute_id = 0;

    size_t position_byte_length = decoderGetAttributeByteLength(decoder, position_attribute_id);
    if (position_byte_length == 0) {
        UtilityFunctions::printerr("Position attribute byte length is zero");
        decoderRelease(decoder);
        return Ref<Mesh>();
    }

    float* position_data = (float*)malloc(position_byte_length);

    if (!decoderReadAttribute(decoder, position_attribute_id, component_type_float, const_cast<char*>("FLOAT"))) {
        UtilityFunctions::printerr("Failed to read position attribute");
        std::free(position_data);
        decoderRelease(decoder);
        return Ref<Mesh>();
    }

    decoderCopyAttribute(decoder, position_attribute_id, position_data);

    PackedVector3Array vertices;
    vertices.resize(vertex_count);

    for (uint32_t i = 0; i < vertex_count; ++i) {
        vertices[i] = Vector3(position_data[i*3], position_data[i*3 + 1], position_data[i*3 + 2]);
    }

    std::free(position_data);

    // Indices

    size_t indices_byte_length = decoderGetIndicesByteLength(decoder);
    if (indices_byte_length == 0) {
        UtilityFunctions::printerr("Indices byte length is zero");
        decoderRelease(decoder);
        return Ref<Mesh>();
    }

    uint32_t* index_data = (uint32_t*)malloc(indices_byte_length);

    if (!decoderReadIndices(decoder, component_type_uint)) {
        UtilityFunctions::printerr("Failed to read indices");
        std::free(index_data);
        decoderRelease(decoder);
        return Ref<Mesh>();
    }

    decoderCopyIndices(decoder, index_data);

    PackedInt32Array indices;
    indices.resize(index_count);
    for (uint32_t i = 0; i < index_count; ++i) {
        indices[i] = index_data[i];
    }

    std::free(index_data);

    // Create Godot mesh
    Ref<ArrayMesh> mesh;
    mesh.instantiate();

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);
    arrays[Mesh::ARRAY_VERTEX] = vertices;
    arrays[Mesh::ARRAY_INDEX] = indices;

    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    decoderRelease(decoder);
    return mesh;
}

bool GDDraco::is_png_from_buffer_view(const PackedByteArray &buffer, const Ref<GLTFState> &p_state) {
    const uint8_t png_signature[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

    // Check size first
    if (buffer.size() < 8) {
        return false;
    }

    // Compare first 8 bytes
    for (int i = 0; i < 8; ++i) {
        if (buffer[i] != png_signature[i]) {
            return false;
        }
    }
    return true;
}
