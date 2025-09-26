#include "GDDraco.hpp"

using namespace godot;

void GDDraco::_bind_methods() {} //only required to allow methods to be called from GDScript

//Default Constructor and destructor
GDDraco::GDDraco() {}
GDDraco::~GDDraco() {}

//Tell Godot that GDDraco supports KHR_draco_mesh_compression
PackedStringArray GDDraco::_get_supported_extensions() {
    //UtilityFunctions::print("GDDraco::_get_supported_extensions called!");
    PackedStringArray extensions;
    extensions.append("KHR_draco_mesh_compression");
    return extensions;
}

//Tell Godot if it should use GDDraco or not
Error GDDraco::_import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) {
    //UtilityFunctions::print("GDDraco::_import_preflight called!");

    for (int i = 0; i < p_extensions.size(); ++i) {
        String ext = p_extensions[i];
        if (ext == "KHR_draco_mesh_compression") {
            return OK; // Proceed with Draco decoding
        }
    }

    return ERR_SKIP; // Skip processing if Draco is not used
}

//Our Importing Logic
Error GDDraco::_import_post_parse(const Ref<GLTFState> &p_state) {
    //UtilityFunctions::print("GDDraco::_import_post_parse called!");

    // Get buffer views from GLTFState
    TypedArray<Ref<GLTFBufferView>> buffer_views = p_state->get_buffer_views();

    //Get the JSON
    Dictionary json = p_state->get_json();
    if (!json.has("meshes")) {
       return ERR_INVALID_PARAMETER; 
    }
    Array arr_meshes = json["meshes"];

    //For each of the meshes decode each of their primitives
    for (int i = 0; i < (int)arr_meshes.size(); i++) {
        Dictionary dic_mesh = arr_meshes[i];

        //Create a vector to have all of the primitives per Mesh
        std::vector<PrimitiveData> vec_primitives;

        //Get the data on mesh primitives
        if (!dic_mesh.has("primitives")) {
            UtilityFunctions::printerr("Skipping mesh " + String::num_int64(i) + " due to no primitives key");
            continue;
        }
        Array arr_primitives = dic_mesh["primitives"];

        //Get Mesh Name
        String mesh_name = "Mesh";
        if (dic_mesh.has("primitives")) {
            mesh_name = dic_mesh["name"];
        }

        //Go through each primitive
        for (int r = 0; r < (int)arr_primitives.size(); r++) {
            Dictionary dic_primitive = arr_primitives[r];

            if (!dic_primitive.has("extensions")) {
                UtilityFunctions::printerr("Skipping primitive " + String::num_int64(r) + " due to no extensions key");
                continue;
            }
            Dictionary dic_extensions = dic_primitive["extensions"];

            if (!dic_extensions.has("KHR_draco_mesh_compression")) {
                UtilityFunctions::printerr("Skipping primitive " + String::num_int64(r) + " due to no KHR_draco_mesh_compression key");
                continue;
            }
            Dictionary dic_KHR_draco_mesh_compression = dic_extensions["KHR_draco_mesh_compression"];
            if (!dic_KHR_draco_mesh_compression.has("bufferView")) {
                UtilityFunctions::printerr("Skipping primitive " + String::num_int64(r) + " due to no bufferView key");
                continue;
            }
            int bufferViewIdx = dic_KHR_draco_mesh_compression["bufferView"];

            Ref<GLTFBufferView> buffer_view = buffer_views[bufferViewIdx];
            int byte_length = buffer_view->get_byte_length();

            //Verify if buffer is valid
            PackedByteArray buffer = buffer_view->load_buffer_view_data(p_state);
            if (buffer.size() != byte_length) {
                UtilityFunctions::printerr("bufferView length mismatch (expected: ", byte_length, ", actual: ", buffer.size(), ")");
                return ERR_INVALID_DATA;
            }

            if (!dic_KHR_draco_mesh_compression.has("attributes")) {
                UtilityFunctions::printerr("Skipping primitive " + String::num_int64(r) + " due to no attributes key");
                continue;
            }
            Dictionary dic_attributes = dic_KHR_draco_mesh_compression["attributes"];

            //GET ATTRIBUTES DATA
            if (!dic_attributes.has("POSITION")) {
                UtilityFunctions::printerr("Skipping primitive " + String::num_int64(r) + " due to no POSITION key");
                continue;
            }
            int position_id = dic_attributes["POSITION"];

            int normal_id = -1;
            if (dic_attributes.has("NORMAL")) {
                normal_id = dic_attributes["NORMAL"];
            }

            int uv_id = -2;
            if (dic_attributes.has("TEXCOORD_0")) {
                uv_id = dic_attributes["TEXCOORD_0"];
            }

            int joints_id = -3;
            if (dic_attributes.has("JOINTS_0")) {
                joints_id = dic_attributes["JOINTS_0"];
            }
            
            int weights_id = -4;
            if (dic_attributes.has("WEIGHTS_0")) {
                weights_id = dic_attributes["WEIGHTS_0"];
            }

            if (!dic_primitive.has("indices")) {
                UtilityFunctions::printerr("Skipping primitive " + String::num_int64(r) + " due to no indices key");
                continue;
            }
            int indices_id = dic_primitive["indices"];

            int material_Idx = -5;
            if (dic_primitive.has("material")) {
                material_Idx = dic_primitive["material"];
            }

            //Decode Mesh
            Ref<ArrayMesh> primitive = decode_draco_mesh(buffer, position_id, normal_id, uv_id, joints_id, weights_id, indices_id);
            if (primitive == nullptr) {
                return ERR_INVALID_DATA; 
            }
            //UtilityFunctions::print("Primitive Decoded!");

            PrimitiveData primitive_data = PrimitiveData(material_Idx, primitive);

            vec_primitives.push_back(primitive_data);
        }

        //Assign the mesh data so that it appears in godot
        TypedArray<Ref<GLTFMesh>> meshes_mesh = p_state->get_meshes();
        TypedArray<Ref<Material>> meshes_materials = p_state->get_materials();
        if (i >= 0 && i < meshes_mesh.size()) {
            //Create Importer Mesh
            Ref<ImporterMesh> importer_mesh;
            importer_mesh.instantiate();

            //Add all primitives to this ImporterMesh
            for (int t = 0; t < (int)vec_primitives.size(); t++) {
                PrimitiveData prim = vec_primitives[t];
                importer_mesh = add_primitive_to_importer_mesh(prim.primitive, importer_mesh);

                if (prim.material_Idx >= 0) {
                    Ref<Material> mat = meshes_materials[prim.material_Idx];
                    importer_mesh->set_surface_material(t, mat);
                }

                importer_mesh->set_surface_name(t, mesh_name);
            }

            //UtilityFunctions::print("Created ImpoterMesh!");
            Ref<GLTFMesh> mesh_to_change = meshes_mesh[i];
            mesh_to_change->set_original_name(mesh_name);
            mesh_to_change->set_mesh(importer_mesh);
            //UtilityFunctions::print("Mesh is set!");
        }
    }

    return OK;
}

//Adds the passed primitive to the importer_mesh passsed
Ref<ImporterMesh> GDDraco::add_primitive_to_importer_mesh(const Ref<ArrayMesh> &source_mesh, Ref<ImporterMesh> importer_mesh) {
	if (source_mesh.is_null()) {
		return importer_mesh;
	}

    // Get number of blend shapes and surfaces (Same for all ArrayMeshes)
    int blend_shape_count = source_mesh->get_blend_shape_count();
	const int surface_count = source_mesh->get_surface_count();


    // Register blend shape names (once per primitive)
    if (blend_shape_count > 0 && importer_mesh->get_blend_shape_count() == 0) {
        for (int b = 0; b < blend_shape_count; ++b) {
            String name = source_mesh->get_blend_shape_name(b);
            importer_mesh->add_blend_shape(name);
        }

        // Optional: set blend shape mode (default is normalized)
        importer_mesh->set_blend_shape_mode(Mesh::BLEND_SHAPE_MODE_NORMALIZED);
    }

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

		// 3. Extract blend shapes for this surface
        TypedArray<Array> blend_shapes;
        if (blend_shape_count > 0) {
            blend_shapes = source_mesh->surface_get_blend_shape_arrays(i);
        }

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


// Function that handles calling the Draco Decoder
Ref<ArrayMesh> GDDraco::decode_draco_mesh(const PackedByteArray &compressed_buffer, int position_id, int normal_id, int uv_id, int joints_id, int weights_id, int indices_id) {
    //UtilityFunctions::print("GDDraco::decode_draco_mesh");

    //Verify if buffer ids are different
    std::set<int> buffer_ids = {position_id, normal_id, uv_id, joints_id, weights_id, indices_id};
    if (buffer_ids.size() < 6) {
        ERR_FAIL_COND_V_MSG(true, nullptr, "One or more invalid buffer ids.");
        return nullptr;
    }

    //Set Up decoder
    Decoder *decoder = decoderCreate();
    if (!decoder) {
        ERR_FAIL_COND_V_MSG(true, nullptr, "Failed to create Draco decoder");
        return nullptr;
    }

    //Decode compressed buffer
    if (compressed_buffer.size() < 32) {
        decoderRelease(decoder);
        ERR_FAIL_V_MSG(nullptr, "Compressed buffer too small");
        return nullptr;
    }
    if (!decoderDecode(decoder, (void *)compressed_buffer.ptr(), compressed_buffer.size())) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "Failed to decode Draco buffer");
        return nullptr;
    }

    //Get vertex and index count
    uint32_t vertex_count = decoderGetVertexCount(decoder);
    uint32_t index_count = decoderGetIndexCount(decoder);
    if (vertex_count == 0 || index_count == 0) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "Decoded mesh has zero vertices or indices");
        return nullptr;
    }

    //Create necessary variables
    PackedVector3Array positions;
    PackedVector3Array normals;
    PackedVector2Array uvs;
    PackedInt32Array joints;
    PackedFloat32Array weights;

    normals.resize(static_cast<int64_t>(vertex_count));
    uvs.resize(static_cast<int64_t>(vertex_count));

    const int64_t joint_element_count = static_cast<int64_t>(vertex_count) * 4;
    joints.resize(joint_element_count);
    
    const int64_t weight_element_count = static_cast<int64_t>(vertex_count) * 4;
    weights.resize(weight_element_count);

    // Decode POSITION (required) 
    if (position_id < 0) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "No Position buffer in current mesh. Please provide a valid GLTF to decode.");
    }
    positions.resize(static_cast<int64_t>(vertex_count));

    if (!decoderReadAttribute(decoder, position_id, 5126, "VEC3")) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "Failed to decode POSITION attribute");
        return nullptr;
    }
    void *test_ptr = positions.ptrw();
    if (!test_ptr) {
        decoderRelease(decoder);
        ERR_FAIL_COND_V_MSG(true, nullptr, "positions.ptrw() is NULL");
        return nullptr;
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
    // Allocate temporary raw data buffer for joints (uint16_t, 2 bytes each)
    PackedByteArray raw_joint_data;
    raw_joint_data.resize(joint_element_count * 2); // 2 bytes per uint16_t

    if (joints_id >= 0 && decoderReadAttribute(decoder, joints_id, 5123, "VEC4")) {
        decoderCopyAttribute(decoder, joints_id, raw_joint_data.ptrw());

        // Pointers to source (uint16_t) and destination (int32_t) arrays
        const uint16_t *src_joint = reinterpret_cast<const uint16_t *>(raw_joint_data.ptr());
        int32_t *dst = joints.ptrw();

        // Convert each uint16_t joint index to int32_t explicitly
        for (int64_t i = 0; i < joint_element_count; i++) {
            dst[i] = static_cast<int32_t>(src_joint[i]);
        }
    } else {
        joints.resize(0);
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
        return nullptr;
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

    if (positions.size() == vertex_count) {
        arrays[Mesh::ARRAY_VERTEX] = positions;
    } else {
        ERR_FAIL_COND_V_MSG(true, nullptr, "Invalid positions. Please provide a valid GLTF to decode.");
        return nullptr;
    }
    if (normal_id > 0 && normals.size() == vertex_count) {
        arrays[Mesh::ARRAY_NORMAL] = normals;
    } else {
        UtilityFunctions::print("Failed to set Primitive's Normals");
    }
    if (uv_id > 0 && uvs.size() == vertex_count) {
        arrays[Mesh::ARRAY_TEX_UV] = uvs;
    } else {
        UtilityFunctions::print("Failed to set Primitive's UV");
    }
    if (joints_id > 0 && joints.size() == joint_element_count) {
        arrays[Mesh::ARRAY_BONES] = joints;
    } else {
        UtilityFunctions::print("Failed to set Primitive's Joints/Bones");
    }
    if (weights_id > 0 && weights.size() == weight_element_count) {
        arrays[Mesh::ARRAY_WEIGHTS] = weights;
    } else {
        UtilityFunctions::print("Failed to set Primitive's Normals");
    }
    if (indices.size() == index_count) {
        arrays[Mesh::ARRAY_INDEX] = indices;
    } else {
        ERR_FAIL_COND_V_MSG(true, nullptr, "Invalid Indices. Please provide a valid GLTF to decode.");
        return nullptr;
    }

    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);

    return mesh;
}
