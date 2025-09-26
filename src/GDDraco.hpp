#ifndef GD_DRACO_HPP
#define GD_DRACO_HPP

#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/gltf_document_extension.hpp>
#include <godot_cpp/classes/gltf_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/importer_mesh.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/gltf_buffer_view.hpp>

#include <src/decoder.h>

#include <cstdint>
#include <cstring>

namespace godot {
    class GDDraco: public GLTFDocumentExtension {
        GDCLASS(GDDraco,GLTFDocumentExtension);

        protected:
            static void _bind_methods();

        public:
            GDDraco();
            ~GDDraco();

            Error _import_post_parse(const Ref<GLTFState> &p_state) override;

            //Used to determine if my extension should be used by GLTF Importer or not
            Error _import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) override;

            //Tell Godot that KHR_draco_mesh_compression is supported
            PackedStringArray _get_supported_extensions();

            //Custom method to connect with Draco Decoder from the Draco Wrapper
            Ref<ArrayMesh> decode_draco_mesh(const PackedByteArray &compressed_buffer, int position_id, int normal_id, int uv_id, int joints_id, int weights_id, int indices_id);

            //Method that grabs the decoded mesh and transforms it into 
            Ref<ImporterMesh> create_importer_mesh_from_array_mesh(const Ref<ArrayMesh> &source_mesh);
    };
}

#endif //GD_DRACO_HPP