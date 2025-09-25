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

Error GDDraco::_import_post_parse(const Ref<GLTFState> &p_state) {
    UtilityFunctions::print("GDDraco::_import_post_parse called!");

    // Get buffer views from GLTFState
    TypedArray<Ref<GLTFBufferView>> buffer_views = p_state->get_buffer_views();

    //Get the JSON
    Dictionary json = p_state->get_json();
    Array meshes = json["meshes"];
    for (int i = 0; i < (int)meshes.size(); i++) {
        Dictionary mesh = meshes[i];
        Array primitives = mesh["primitives"];
        Dictionary primitive = primitives[0];
        Dictionary extensions = primitive["extensions"];
        Dictionary KHR_draco_mesh_compression = extensions["KHR_draco_mesh_compression"];
        int bufferViewIdx = KHR_draco_mesh_compression["bufferView"];

        Ref<GLTFBufferView> buffer_view = buffer_views[bufferViewIdx];
        int byte_offset = buffer_view->get_byte_offset();
        int byte_length = buffer_view->get_byte_length();

        //Verify if buffer is valid
        PackedByteArray buffer = buffer_view->load_buffer_view_data(p_state);
        if (byte_offset < 0 || byte_offset + byte_length > buffer.size()) {
            UtilityFunctions::printerr("bufferView range invalid");
            return ERR_INVALID_PARAMETER;
        }

        Dictionary attributes = KHR_draco_mesh_compression["attributes"];
        int position_id = attributes["POSITION"];
        int normal_id = attributes["NORMAL"];
        int uv_id = attributes["TEXCOORD_0"];
        int joints_id = attributes["JOINTS_0"];
        int weights_id = attributes["WEIGHTS_0"];
        int indices_id = primitive["indices"];

        Ref<ArrayMesh> my_mesh = decode_draco_mesh(buffer, position_id, normal_id, uv_id, joints_id, weights_id, indices_id);
        UtilityFunctions::print("Mesh Decoded?");

        TypedArray<Ref<GLTFMesh>> meshes_mesh = p_state->get_meshes();
        if (i >= 0 && i < meshes_mesh.size()) {
            Ref<ImporterMesh> importerMesh = create_importer_mesh_from_array_mesh(my_mesh);
            UtilityFunctions::print("Created ImpoterMesh?");
            Ref<GLTFMesh> mesh_to_change = meshes_mesh[i];
            mesh_to_change->set_mesh(importerMesh);
            UtilityFunctions::print("Mesh is set?");
        }
    }
    

    /*
    Acessors are required for the decoding: After decoding??????
    Array[GLTFAccessor] get_accessors() 

    https://github.com/blender/blender/blob/97297bd167aed7e05542fbbc85959365539f1e8b/scripts/addons_core/io_scene_gltf2/io/imp/gltf2_io_binary.py
    */

    return OK;
}

PackedStringArray GDDraco::_get_supported_extensions() {
    UtilityFunctions::print("GDDraco::_get_supported_extensions called!");

    PackedStringArray extensions;
    extensions.append("KHR_draco_mesh_compression");
    UtilityFunctions::print(extensions);
    return extensions;
}

Ref<ImporterMesh> GDDraco::create_importer_mesh_from_array_mesh(const Ref<ArrayMesh> &source_mesh) {
	if (source_mesh.is_null()) {
		return Ref<ImporterMesh>();
	}

	Ref<ImporterMesh> importer_mesh;
	importer_mesh.instantiate();

	const int surface_count = source_mesh->get_surface_count();

	for (int i = 0; i < surface_count; ++i) {
		// 1. Get surface data
		Mesh::PrimitiveType primitive_type = source_mesh->surface_get_primitive_type(i);
		Array arrays = source_mesh->surface_get_arrays(i);
		Ref<Material> material = source_mesh->surface_get_material(i);

		// 2. Optional: Get surface name if using names
		String name;
		if (source_mesh->surface_get_name(i) != String()) {
			name = source_mesh->surface_get_name(i);
		}

		// 3. Optional: Get blend shapes if present
		TypedArray<Array> blend_shapes; // TODO: extract blend shapes if needed

		// 4. Add to ImporterMesh
		importer_mesh->add_surface(
			primitive_type,
			arrays,
			blend_shapes,
			Dictionary(), // LODs – not used here
			material,
			name,
			0 // flags
		);
	}

	return importer_mesh;
}



Ref<ArrayMesh> GDDraco::decode_draco_mesh(const PackedByteArray &compressed_buffer, int position_id, int normal_id, int uv_id, int joints_id, int weights_id, int indices_id) {
    UtilityFunctions::print("GDDraco::decode_draco_mesh");
    Decoder *decoder = decoderCreate();

    if (!decoder) {
        ERR_FAIL_COND_V_MSG(true, nullptr, "Failed to create Draco decoder");
    }

    UtilityFunctions::print("Compressed buffer size: " + itos(compressed_buffer.size()));
    if (compressed_buffer.size() < 32) {
        decoderRelease(decoder);
        ERR_FAIL_V_MSG(nullptr, "Compressed buffer too small");
    }

    if (!decoderDecode(decoder, (void *)compressed_buffer.ptr(), compressed_buffer.size())) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "Failed to decode Draco buffer");
    }

    uint32_t vertex_count = decoderGetVertexCount(decoder);
    uint32_t index_count = decoderGetIndexCount(decoder);


    if (vertex_count == 0 || index_count == 0) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "Decoded mesh has zero vertices or indices");
    }

    PackedVector3Array positions;
    PackedVector3Array normals;
    PackedVector2Array uvs;
    PackedInt32Array joints;
    PackedFloat32Array weights;

    positions.resize(static_cast<int64_t>(vertex_count));
    normals.resize(static_cast<int64_t>(vertex_count));
    uvs.resize(static_cast<int64_t>(vertex_count));
    
    const int64_t weight_element_count = static_cast<int64_t>(vertex_count) * 4;
    weights.resize(weight_element_count);

    // Decode POSITION (required)
    if (!decoderReadAttribute(decoder, position_id, 5126, "VEC3")) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "Failed to decode POSITION attribute");
    }
    void *test_ptr = positions.ptrw();
    if (!test_ptr) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "positions.ptrw() is NULL");
    }
    decoderCopyAttribute(decoder, position_id, positions.ptrw());

    // Decode NORMAL (optional)
    if (normal_id >= 0 && decoderReadAttribute(decoder, normal_id, 5126, "VEC3")) {
        decoderCopyAttribute(decoder, normal_id, normals.ptrw());
    } else {
        normals.resize(0);
    }

    // Decode TEXCOORD_0 (optional)
    if (uv_id >= 0 && decoderReadAttribute(decoder, uv_id, 5126, "VEC2")) {
        decoderCopyAttribute(decoder, uv_id, uvs.ptrw());
    } else {
        uvs.resize(0);
    }

    // Decode JOINTS_0 (optional)
    //Get JOINTS to temporary memory
    const int64_t joint_element_count = static_cast<int64_t>(vertex_count) * 4;

    PackedByteArray raw_joint_data;
    raw_joint_data.resize(joint_element_count * 2); // 2 bytes per uint16_t

    if (!decoderReadAttribute(decoder, joints_id, 5123, "VEC4")) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "Failed to decode JOINTS_0 attribute");
    }
    decoderCopyAttribute(decoder, joints_id, raw_joint_data.ptrw());

    //Place joints on correct memory
    joints.resize(joint_element_count);
    const uint16_t *src_joint = reinterpret_cast<const uint16_t *>(raw_joint_data.ptr());
    int32_t *dst = joints.ptrw();

    for (int64_t i = 0; i < joint_element_count; i++) {
        dst[i] = static_cast<int32_t>(src_joint[i]);
    }



    // Decode WEIGHTS_0 (optional)
    if (weights_id >= 0 && decoderReadAttribute(decoder, weights_id, 5126, "VEC4")) {
        decoderCopyAttribute(decoder, weights_id, weights.ptrw());
    } else {
        weights.resize(0);
    }

    // Decode INDICES (required)
    if (!decoderReadIndices(decoder, 5123)) { // 5123 = unsigned short indices
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "Failed to decode indices");
    }

    PackedByteArray raw_indices_16;
    raw_indices_16.resize(index_count * 2);
    decoderCopyIndices(decoder, raw_indices_16.ptrw());

    PackedInt32Array indices;
    indices.resize(index_count);

    const uint16_t *src_idx = reinterpret_cast<const uint16_t *>(raw_indices_16.ptr());
    for (uint32_t i = 0; i < index_count; ++i) {
        indices[i] = static_cast<int32_t>(src_idx[i]);
    }


    decoderRelease(decoder);

    // Now create ArrayMesh
    Ref<ArrayMesh> mesh;
    mesh.instantiate();

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);
    arrays[Mesh::ARRAY_VERTEX] = positions;
    if (normals.size() == vertex_count)
        arrays[Mesh::ARRAY_NORMAL] = normals;
    if (uvs.size() == vertex_count)
        arrays[Mesh::ARRAY_TEX_UV] = uvs;
    if (joints.size() == vertex_count)
        arrays[Mesh::ARRAY_BONES] = joints;
    if (weights.size() == vertex_count)
        arrays[Mesh::ARRAY_WEIGHTS] = weights;
    arrays[Mesh::ARRAY_INDEX] = indices;

    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    return mesh;
}


//_________________________________________________________________________


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

Ref<ArrayMesh> GDDraco::decode_draco_primitive(const PackedByteArray &compressed_data) {
    UtilityFunctions::print("GDDraco::decode_draco_primitive called!");

    Decoder* decoder = decoderCreate();
    if (!decoderDecode(decoder, (void*)compressed_data.ptr(), compressed_data.size())) {
        UtilityFunctions::printerr("Failed to decode Draco primitive");
        decoderRelease(decoder);
        return Ref<ArrayMesh>();
    }

    uint32_t vertex_count = decoderGetVertexCount(decoder);
    uint32_t index_count = decoderGetIndexCount(decoder);

    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);

    // Position
    {
        int position_attr_id = 0;
        if (position_attr_id < 0) {
            UtilityFunctions::printerr("No POSITION attribute found");
            decoderRelease(decoder);
            return Ref<ArrayMesh>();
        }

        size_t pos_bytes = decoderGetAttributeByteLength(decoder, position_attr_id);
        float* pos_data = (float*)malloc(pos_bytes);

        decoderReadAttribute(decoder, position_attr_id, 5126, "VEC3");
        decoderCopyAttribute(decoder, position_attr_id, pos_data);

        PackedVector3Array positions;
        positions.resize(vertex_count);
        for (uint32_t i = 0; i < vertex_count; ++i) {
            positions[i] = Vector3(pos_data[i * 3], pos_data[i * 3 + 1], pos_data[i * 3 + 2]);
        }

        arrays[Mesh::ARRAY_VERTEX] = positions;
        std::free(pos_data);
    }

    // Normals (optional)
    {
        int normal_attr_id = 1;
        if (normal_attr_id >= 0) {
            size_t bytes = decoderGetAttributeByteLength(decoder, normal_attr_id);
            float* data = (float*)malloc(bytes);
            decoderReadAttribute(decoder, normal_attr_id, 5126, "VEC3");
            decoderCopyAttribute(decoder, normal_attr_id, data);

            PackedVector3Array normals;
            normals.resize(vertex_count);
            for (uint32_t i = 0; i < vertex_count; ++i) {
                normals[i] = Vector3(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
            }
            arrays[Mesh::ARRAY_NORMAL] = normals;
            std::free(data);
        }
    }

    // UVs (optional)
    {
        int uv_attr_id = 2;
        if (uv_attr_id >= 0) {
            size_t bytes = decoderGetAttributeByteLength(decoder, uv_attr_id);
            float* data = (float*)malloc(bytes);
            decoderReadAttribute(decoder, uv_attr_id, 5126, "VEC2");
            decoderCopyAttribute(decoder, uv_attr_id, data);

            PackedVector2Array uvs;
            uvs.resize(vertex_count);
            for (uint32_t i = 0; i < vertex_count; ++i) {
                uvs[i] = Vector2(data[i * 2], data[i * 2 + 1]);
            }
            arrays[Mesh::ARRAY_TEX_UV] = uvs;
            std::free(data);
        }
    }

    // Indices
    {
        size_t indices_bytes = decoderGetIndicesByteLength(decoder);
        uint32_t* idx_data = (uint32_t*)malloc(indices_bytes);

        decoderReadIndices(decoder, 5125); // Unsigned int
        decoderCopyIndices(decoder, idx_data);

        PackedInt32Array indices;
        indices.resize(index_count);
        for (uint32_t i = 0; i < index_count; ++i) {
            indices[i] = idx_data[i];
        }

        arrays[Mesh::ARRAY_INDEX] = indices;
        std::free(idx_data);
    }

    Ref<ArrayMesh> mesh;
    mesh.instantiate();
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    decoderRelease(decoder);
    return mesh;
}