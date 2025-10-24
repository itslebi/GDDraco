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

#ifndef GD_DRACO_HPP
#define GD_DRACO_HPP

#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/gltf_document_extension.hpp>
#include <godot_cpp/classes/gltf_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/importer_mesh.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/gltf_buffer_view.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

#include <src/decoder.h>

#include <cstdint>
#include <cstdlib>

#include <vector>

#include "PrimitiveData.hpp"
#include "AttributeStorer.hpp"
#include "headers/logging.hpp"
#include "headers/templates.hpp"

namespace godot {
    class GDDraco: public GLTFDocumentExtension {
        GDCLASS(GDDraco,GLTFDocumentExtension);

        private:
            //Helper Method to handle weights
            void decode_and_normalize_weights(const PackedByteArray &raw_data, int vertex_count, int components_per_vertex, int comp_type, PackedFloat32Array &out_result);

            //Helper Method to handle color
            void decode_normalized_color(const PackedByteArray &raw_data, int vertex_count, int components_per_vertex, int comp_type, PackedColorArray &out_colors);

        protected:
            static void _bind_methods();

            //Custom method to connect with Draco Decoder from the Draco Wrapper
            Ref<ArrayMesh> decode_draco_mesh(const PackedByteArray &compressed_buffer, int indices_id, std::vector<AttributeStorer> &vec_attr);

            //Method that grabs the decoded mesh and adds it to an ImporterMesh
            Ref<ImporterMesh> add_primitive_to_importer_mesh(const Ref<ArrayMesh> &source_mesh, Ref<ImporterMesh> importer_mesh);

        public:
            GDDraco();
            ~GDDraco();

            //This is where our decoding logic happens
            Error _import_post_parse(const Ref<GLTFState> &p_state) override;

            //Used to determine if my extension should be used by GLTF Importer or not
            Error _import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) override;

            //Tell Godot that KHR_draco_mesh_compression is supported
            PackedStringArray _get_supported_extensions() override;
    };
}

#endif //GD_DRACO_HPP