#include "GDDraco.hpp"

#include <cstdlib>

using namespace godot;

void GDDraco::_bind_methods() {
    // No binding needed unless exposing to GDScript.
}

GDDraco::GDDraco() {

}

GDDraco::~GDDraco() {

}

Error GDDraco::_import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) {
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

    // TODO: pass draco_data to your Draco decoder here
    Ref<Mesh> decoded_mesh = decode_draco_mesh(draco_data);
    if (decoded_mesh.is_null()) {
        UtilityFunctions::printerr("Failed to decode Draco mesh");
        return ERR_CANT_CREATE;
    }

    // TODO: attach decoded_mesh to p_gltf_node or the scene graph as needed

    UtilityFunctions::print("Draco mesh decoded successfully");
    return OK;
}

PackedStringArray GDDraco::_get_supported_extensions() {
    PackedStringArray extensions;
    extensions.append("KHR_draco_mesh_compression");
    return extensions;
}


Ref<Mesh> GDDraco::decode_draco_mesh(const PackedByteArray &compressed_data) {
    Decoder* decoder = decoderCreate();
    if (!decoderDecode(decoder, (void*)compressed_data.ptr(), compressed_data.size())) {
        UtilityFunctions::printerr("Failed to decode Draco mesh");
        decoderRelease(decoder);
        return Ref<Mesh>();
    }

    // Extract vertex count and index count
    uint32_t vertex_count = decoderGetVertexCount(decoder);
    uint32_t index_count = decoderGetIndexCount(decoder);

    // TODO: You need to figure out component types from GLTF accessor info (e.g., float for positions)
    // For example, positions, normals, uvs might be floats, indices usually unsigned short or uint32

    // Allocate arrays for vertices and indices
    PackedVector3Array vertices;
    vertices.resize(vertex_count);

    PackedInt32Array indices;
    indices.resize(index_count);

    // Assuming you know attribute ids for POSITION and indices component type
    // For POSITION attribute:
    uint32_t position_attribute_id = 0; // Replace with real ID from Draco mesh
    if (!decoderReadAttribute(decoder, position_attribute_id, ComponentType::Float, "FLOAT")) {
        UtilityFunctions::printerr("Failed to read position attribute");
        decoderRelease(decoder);
        return Ref<Mesh>();
    }

    // Copy decoded position data into vertices array
    // Your decoder stores raw bytes, cast and copy accordingly
    float* position_data = (float*)malloc(sizeof(float) * vertex_count * 3);
    decoderCopyAttribute(decoder, position_attribute_id, position_data);

    for (uint32_t i = 0; i < vertex_count; ++i) {
        vertices[i] = Vector3(position_data[i * 3], position_data[i * 3 + 1], position_data[i * 3 + 2]);
    }
    std::free(position_data);

    // Similarly for indices:
    // For example, assuming indices are unsigned int
    decoderReadIndices(decoder, ComponentType::UnsignedInt);
    uint32_t* index_data = (uint32_t*)malloc(sizeof(uint32_t) * index_count);
    decoderCopyIndices(decoder, index_data);
    for (uint32_t i = 0; i < index_count; ++i) {
        indices[i] = index_data[i];
    }
    std::free(index_data);

    // Create Godot ArrayMesh and fill with data
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

