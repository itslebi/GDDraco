#ifndef GD_DRACO_HPP
#define GD_DRACO_HPP

#include <godot_cpp/classes/gltf_node.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/gltf_document_extension.hpp>
#include <godot_cpp/classes/gltf_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/gltf_buffer_view.hpp>

#include <src/decoder.h>
#include <draco/compression/decode.h>
#include <draco/compression/mesh/mesh_decoder.h>
#include <draco/core/decoder_buffer.h>
#include <draco/mesh/mesh.h>
#include <draco/core/status.h>
#include <draco/point_cloud/point_cloud.h>

namespace godot {
    class GDDraco: public GLTFDocumentExtension {
        GDCLASS(GDDraco,GLTFDocumentExtension);

        protected:
            static void _bind_methods();

        public:
            GDDraco();
            ~GDDraco();
    
            // Override this method to handle mesh compression
            Error _parse_node_extensions(const Ref<GLTFState> &p_state, const Ref<GLTFNode> &p_gltf_node, const Dictionary &p_extensions) override;

            //Used to determine if my extension should be used by GLTF Importer or not
            Error _import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) override;

            //Tell Godot that KHR_draco_mesh_compression is supported
            PackedStringArray _get_supported_extensions();

            //Custom method to connect with Draco Decoder from the Draco Wrapper
            Ref<Mesh> GDDraco::decode_draco_mesh(const PackedByteArray &compressed_data);
    };
}

#endif //GD_DRACO_HPP