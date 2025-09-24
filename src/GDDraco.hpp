#ifndef GD_DRACO_HPP
#define GD_DRACO_HPP

#include <godot_cpp/classes/gltf_node.hpp>
#include <godot_cpp/classes/gltf_state.hpp>
#include <godot_cpp/classes/gltf_document_extension.hpp>
#include <godot_cpp/classes/gltf_object_model_property.hpp>
#include <godot_cpp/classes/gltf_mesh.hpp>
#include <godot_cpp/classes/mesh.hpp>
#include <godot_cpp/classes/importer_mesh.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/classes/gltf_buffer_view.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/gltf_texture.hpp>
#include <godot_cpp/classes/image.hpp>

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

            Ref<GLTFObjectModelProperty> _import_object_model_property(const Ref<GLTFState> &p_state, const PackedStringArray &p_split_json_pointer, const TypedArray<NodePath> &p_partial_paths) override;
    
            // Override this method to handle mesh compression ???????????
            Error _parse_node_extensions(const Ref<GLTFState> &p_state, const Ref<GLTFNode> &p_gltf_node, const Dictionary &p_extensions) override;

            //Used to determine if my extension should be used by GLTF Importer or not
            Error _import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) override;

            //Tell Godot that KHR_draco_mesh_compression is supported
            PackedStringArray _get_supported_extensions();

            //Custom method to connect with Draco Decoder from the Draco Wrapper
            Ref<Mesh> GDDraco::decode_draco_mesh(const PackedByteArray &compressed_data);
        
        private:
            bool is_png(const uint8_t* buffer, size_t size);
    };
}

#endif //GD_DRACO_HPP