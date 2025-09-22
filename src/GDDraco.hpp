#ifndef GD_DRACO_HPP
#define GD_DRACO_HPP

#include <godot_cpp/classes/gltf_node.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/gltf_document_extension.hpp>
#include <godot_cpp/classes/gltf_mesh.hpp>

#include <draco/compression/decode.h>
#include <draco/compression/mesh/mesh_decoder.h>
#include <draco/core/decoder_buffer.h>
#include <draco/mesh/mesh.h>

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

            Error _import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) override;

            PackedStringArray _get_supported_extensions();
    };
}

#endif //GD_DRACO_HPP