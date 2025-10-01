/*
 * MIT License
 *
 * Copyright (c) 2025 itslebi
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
            gddraco::log_info("KHR_draco_mesh_compression extension found. Using GDDraco.");
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
    TypedArray<Ref<GLTFAccessor>> acessors = p_state->get_accessors();

    //Get the JSON
    Dictionary json = p_state->get_json();
    if (!json.has("meshes")) {
        gddraco::log_error("JSON does not contain 'meshes' key. Invalid GLTF file.");
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
            gddraco::log_warn("Skipping mesh " + String::num_int64(i) + " due to missing 'primitives' key");
            continue;
        }
        Array arr_primitives = dic_mesh["primitives"];

        //Get Mesh Name
        String mesh_name = "Mesh";
        if (!dic_mesh.has("name")) {
            gddraco::log_warn("No name found for this primitive. Using default name `Mesh`.");
        } else {
            mesh_name = dic_mesh["name"];
        }

        //Go through each primitive
        for (int r = 0; r < (int)arr_primitives.size(); r++) {
            Dictionary dic_primitive = arr_primitives[r];

            //Get Ids for Attribute
            if (!dic_primitive.has("attributes")) {
                gddraco::log_warn("Skipping primitive " + String::num_int64(r) + " due to missing 'attributes' key");
                continue;
            }
            Dictionary dic_attributes = dic_primitive["attributes"];

            //Get Ids for Buffer
            if (!dic_primitive.has("extensions")) {
                gddraco::log_warn("Skipping primitive " + String::num_int64(r) + " due to missing 'extensions' key");
                continue;
            }
            Dictionary dic_extensions = dic_primitive["extensions"];

            if (!dic_extensions.has("KHR_draco_mesh_compression")) {
                gddraco::log_warn("Skipping primitive " + String::num_int64(r) + " due to missing 'KHR_draco_mesh_compression' key");
                continue;
            }
            Dictionary dic_KHR_draco_mesh_compression = dic_extensions["KHR_draco_mesh_compression"];

            if (!dic_KHR_draco_mesh_compression.has("bufferView")) {
                gddraco::log_warn("Skipping primitive " + String::num_int64(r) + " due to missing 'bufferView' key");
                continue;
            }
            int bufferViewIdx = dic_KHR_draco_mesh_compression["bufferView"];

            Ref<GLTFBufferView> buffer_view = buffer_views[bufferViewIdx];
            int byte_length = buffer_view->get_byte_length();

            //Verify if buffer is valid
            PackedByteArray buffer = buffer_view->load_buffer_view_data(p_state);
            if (buffer.size() != byte_length) {
                gddraco::log_error("bufferView length mismatch (expected: " + String::num_int64(byte_length) + ", actual: " + String::num_int64(buffer.size()) + ")");
                return ERR_INVALID_DATA;
            }

            if (!dic_KHR_draco_mesh_compression.has("attributes")) {
                gddraco::log_warn("Skipping primitive " + String::num_int64(r) + " due to missing 'attributes' key");
                continue;
            }
            Dictionary dic_KHR_attributes = dic_KHR_draco_mesh_compression["attributes"];
            //Get Attributes data
            Array attribute_keys = dic_KHR_attributes.keys();

            std::vector<AttributeStorer> vec_attr;

            for (int i = 0; i < attribute_keys.size(); i++) {
                String semantic = attribute_keys[i];
                int draco_attribute_id = dic_KHR_attributes[semantic];
                if (!dic_attributes.has(semantic)) {
                    gddraco::log_error("No attribute " + semantic + " found. File Error!");
                }
                int acessor_IDX = dic_attributes[semantic];
                Ref<GLTFAccessor> acc = acessors[acessor_IDX];
                AttributeStorer a = AttributeStorer(semantic, acc->get_component_type(), acc->get_accessor_type(), draco_attribute_id,
                                                    acc->get_count(), acc->get_normalized());

                vec_attr.push_back(a);
            }


            //____________________________________________

            //GET KHR ATTRIBUTES DATA
            if (!dic_KHR_attributes.has("POSITION")) {
                gddraco::log_warn("Skipping primitive " + String::num_int64(r) + " due to missing 'POSITION' attribute");
                continue;
            }

            if (!dic_primitive.has("indices")) {
                gddraco::log_warn("Skipping primitive " + String::num_int64(r) + " due to missing 'indices' key");
                continue;
            }
            int indices_id = dic_primitive["indices"];

            int material_Idx = -2;
            if (dic_primitive.has("material")) {
                material_Idx = dic_primitive["material"];
            } else {
                gddraco::log_info("No material for primitive " + String::num_int64(r) + ". Proceding with default value.");
            }

            //Decode Mesh
            Ref<ArrayMesh> primitive = decode_draco_mesh(buffer, indices_id, vec_attr);
            if (primitive == nullptr) {
                gddraco::log_error("Failed to decode Draco primitive at index " + String::num_int64(r));
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

            Ref<GLTFMesh> mesh_to_change = meshes_mesh[i];
            mesh_to_change->set_original_name(mesh_name);
            mesh_to_change->set_mesh(importer_mesh);
            gddraco::log_info("Mesh imported successfully.");
        }
    }

    return OK;
}

//Adds the passed primitive to the importer_mesh passsed
Ref<ImporterMesh> GDDraco::add_primitive_to_importer_mesh(const Ref<ArrayMesh> &source_mesh, Ref<ImporterMesh> importer_mesh) {
	if (source_mesh.is_null()) {
        gddraco::log_warn("Source mesh is null in add_primitive_to_importer_mesh.");
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

void GDDraco::decode_and_normalize_weights(const PackedByteArray &raw_data, int vertex_count, int components_per_vertex, int comp_type, PackedFloat32Array &out_result) {
    int64_t element_count = static_cast<int64_t>(vertex_count) * components_per_vertex;
    out_result.resize(element_count);

    float *dst = out_result.ptrw();

    switch (comp_type) {
        case 5121: { // uint8
            const uint8_t *src = reinterpret_cast<const uint8_t *>(raw_data.ptr());
            for (int64_t i = 0; i < element_count; ++i) {
                dst[i] = gddraco::normalize_component<uint8_t>(src[i], 255);
            }
            break;
        }
        case 5123: { // uint16
            const uint16_t *src = reinterpret_cast<const uint16_t *>(raw_data.ptr());
            for (int64_t i = 0; i < element_count; ++i) {
                dst[i] = gddraco::normalize_component<uint16_t>(src[i], 65535);
            }
            break;
        }
        default:
            out_result.resize(0);
            gddraco::log_error("Unsupported WEIGHTS component type: " + godot::String::num_int64(comp_type));
            break;
    }
}

void GDDraco::decode_normalized_color(const PackedByteArray &raw_data, int vertex_count, int components_per_vertex, int comp_type, PackedColorArray &out_colors) {
    out_colors.resize(vertex_count);
    Color *dst = out_colors.ptrw();

    if (comp_type == 5121) {
        const uint8_t *src = reinterpret_cast<const uint8_t *>(raw_data.ptr());
        for (int64_t i = 0; i < vertex_count; ++i) {
            float r = components_per_vertex > 0 ? src[i * components_per_vertex + 0] / 255.0f : 0.0f;
            float g = components_per_vertex > 1 ? src[i * components_per_vertex + 1] / 255.0f : 0.0f;
            float b = components_per_vertex > 2 ? src[i * components_per_vertex + 2] / 255.0f : 0.0f;
            float a = components_per_vertex > 3 ? src[i * components_per_vertex + 3] / 255.0f : 1.0f;
            dst[i] = Color(r, g, b, a);
        }
    } else if (comp_type == 5123) {
        const uint16_t *src = reinterpret_cast<const uint16_t *>(raw_data.ptr());
        for (int64_t i = 0; i < vertex_count; ++i) {
            float r = components_per_vertex > 0 ? src[i * components_per_vertex + 0] / 65535.0f : 0.0f;
            float g = components_per_vertex > 1 ? src[i * components_per_vertex + 1] / 65535.0f : 0.0f;
            float b = components_per_vertex > 2 ? src[i * components_per_vertex + 2] / 65535.0f : 0.0f;
            float a = components_per_vertex > 3 ? src[i * components_per_vertex + 3] / 65535.0f : 1.0f;
            dst[i] = Color(r, g, b, a);
        }
    } else {
        out_colors.resize(0); // Unsupported type
    }
}


Ref<ArrayMesh> GDDraco::decode_draco_mesh(const PackedByteArray &compressed_buffer, int indices_id, std::vector<AttributeStorer> &vec_attr) {
    //UtilityFunctions::print("GDDraco::decode_draco_mesh");

    //Set Up decoder
    Decoder *decoder = decoderCreate();
    if (!decoder) {
        gddraco::log_error("Failed to create Draco decoder.");
        return nullptr;
    }

    //Decode compressed buffer
    if (compressed_buffer.size() < 32) {
        decoderRelease(decoder);
        gddraco::log_error("Compressed buffer too small.");
        return nullptr;
    }
    if (!decoderDecode(decoder, (void *)compressed_buffer.ptr(), compressed_buffer.size())) {
        decoderRelease(decoder);
        gddraco::log_error("Failed to decode Draco buffer.");
        return nullptr;
    }

    //Get vertex and index count
    uint32_t vertex_count = decoderGetVertexCount(decoder);
    uint32_t index_count = decoderGetIndexCount(decoder);
    if (vertex_count == 0 || index_count == 0) {
        decoderRelease(decoder);
        gddraco::log_error("Decoded mesh has zero vertices or indices.");
        return nullptr;
    }

    // Predeclare array storage for mesh attributes
    Array arrays;
    arrays.resize(Mesh::ARRAY_MAX);

    // Iterate through vec_attr to decode and assign attributes dynamically
    for (const AttributeStorer &attr : vec_attr) {
        const godot::String &name = attr.name;
        int id = attr.draco_ID;
        int count = attr.count;
        bool normalized = attr.normalized;
        int comp_type = attr.get_comp_type();
        char *acc_type = attr.get_acc_type();
        int components_per_vertex = gddraco::get_component_count(acc_type);

        if (id < 0) {
            gddraco::log_warn("Attribute ID is invalid for: " + name);
            continue;
        }

        // Decode attribute
        if (!decoderReadAttribute(decoder, id, comp_type, acc_type)) {
            gddraco::log_warn("Failed to decode attribute: " + name);
            continue;
        }

        // Match name to appropriate mesh array slot
        if (name == "POSITION") {
            PackedVector3Array positions;
            positions.resize(static_cast<int64_t>(vertex_count));
            decoderCopyAttribute(decoder, id, positions.ptrw());
            arrays[Mesh::ARRAY_VERTEX] = positions;
        } else if (name == "NORMAL") {
            PackedVector3Array normals;
            normals.resize(static_cast<int64_t>(vertex_count));
            gddraco::decoderCopyAttributeVec3f(decoder, id, normals.ptrw());
            arrays[Mesh::ARRAY_NORMAL] = normals;
        } else if (name.begins_with("TEXCOORD_")) {
            int index = name.get_slicec('_', 1).to_int();
            PackedVector2Array uvs;
            uvs.resize(static_cast<int64_t>(vertex_count));
            decoderCopyAttribute(decoder, id, uvs.ptrw());
            arrays[Mesh::ARRAY_TEX_UV + index] = uvs;
        } else if (name == "JOINTS_0") {
            int element_count = static_cast<int64_t>(vertex_count) * components_per_vertex;

            PackedInt32Array joints;
            joints.resize(element_count);

            // Calculate raw data size based on comp_type size (5121 = 1 byte, 5123 = 2 bytes, etc.)
            int comp_size_bytes = 0;
            switch (comp_type) {
                case 5121: comp_size_bytes = 1; break;  // GL_UNSIGNED_BYTE
                case 5123: comp_size_bytes = 2; break;  // GL_UNSIGNED_SHORT
                case 5125: comp_size_bytes = 4; break;  // GL_UNSIGNED_INT
                default:
                    gddraco::log_error("Unsupported JOINTS component type");
                    continue;  // skip this attribute
            }

            PackedByteArray raw_joint_data;
            raw_joint_data.resize(element_count * comp_size_bytes);

        decoderCopyAttribute(decoder, id, raw_joint_data.ptrw());

        int32_t *dst = joints.ptrw();

        if (comp_type == 5121) {
            const uint8_t* src = reinterpret_cast<const uint8_t*>(raw_joint_data.ptr());
            for (int64_t i = 0; i < element_count; i++) {
                dst[i] = static_cast<int32_t>(src[i]);
            }
        } else if (comp_type == 5123) {
            const uint16_t* src = reinterpret_cast<const uint16_t*>(raw_joint_data.ptr());
            for (int64_t i = 0; i < element_count; i++) {
                dst[i] = static_cast<int32_t>(src[i]);
            }
        } else if (comp_type == 5125) {
            const uint32_t* src = reinterpret_cast<const uint32_t*>(raw_joint_data.ptr());
            for (int64_t i = 0; i < element_count; i++) {
                dst[i] = static_cast<int32_t>(src[i]);
            }
        }

        arrays[Mesh::ARRAY_BONES] = joints;
        } else if (name == "WEIGHTS_0") {
            int comp_size_bytes = 0;
            switch (comp_type) {
                case 5121: comp_size_bytes = 1; break;  // GL_UNSIGNED_BYTE
                case 5123: comp_size_bytes = 2; break;  // GL_UNSIGNED_SHORT
                case 5126: comp_size_bytes = 4; break;  // GL_FLOAT
                default:
                    gddraco::log_error("Unsupported WEIGHTS component type");
                    continue;  // skip this attribute
            }
            if (normalized && (comp_type == 5121 || comp_type == 5123)) {
                PackedByteArray raw;
                raw.resize(vertex_count * components_per_vertex * comp_size_bytes);
                decoderCopyAttribute(decoder, id, raw.ptrw());

                PackedFloat32Array weights;
                decode_and_normalize_weights(raw, vertex_count, components_per_vertex, comp_type, weights);
                arrays[Mesh::ARRAY_WEIGHTS] = weights;
            } else {
                PackedFloat32Array weights;
                weights.resize(vertex_count * components_per_vertex);
                decoderCopyAttribute(decoder, id, weights.ptrw());
                arrays[Mesh::ARRAY_WEIGHTS] = weights;
            }
        } else if (name.begins_with("COLOR_0")) {
            PackedColorArray colors;
            if (comp_type == 5126) { // float
                colors.resize(vertex_count);
                decoderCopyAttribute(decoder, id, colors.ptrw());
            } else if (normalized && (comp_type == 5121 || comp_type == 5123)) {
                int bytes_per_component = (comp_type == 5121) ? 1 : 2;
                PackedByteArray raw;
                raw.resize(vertex_count * components_per_vertex * bytes_per_component);
                decoderCopyAttribute(decoder, id, raw.ptrw());

                decode_normalized_color(raw, vertex_count, components_per_vertex, comp_type, colors);
            } else {
                gddraco::log_warn("Unsupported COLOR_0 format: comp_type = " + godot::String::num_int64(comp_type));
                continue;
            }
            arrays[Mesh::ARRAY_COLOR] = colors;
        } else {
            gddraco::log_warn("Unhandled attribute name: " + name + ". Attribute not supported!");
        }
    }

    // Decode INDICES (required)
    if (!decoderReadIndices(decoder, 5123)) { // 5123 = unsigned short indices
        decoderRelease(decoder);
        gddraco::log_error("Failed to decode indices");
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
    arrays[Mesh::ARRAY_INDEX] = indices;

    decoderRelease(decoder);

    Ref<ArrayMesh> mesh;
    mesh.instantiate();
    mesh->add_surface_from_arrays(Mesh::PRIMITIVE_TRIANGLES, arrays);
    return mesh;
}

