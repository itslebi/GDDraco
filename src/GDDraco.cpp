#include "GDDraco.hpp"

using namespace godot;

void GDDraco::_bind_methods() {

}

GDDraco::GDDraco() {

}

GDDraco::~GDDraco() {

}

Error GDDraco::_parse_node_extensions(const Ref<GLTFState> &p_state, const Ref<GLTFNode> &p_gltf_node, const Dictionary &p_extensions) {
    UtilityFunctions::print("GDDraco::_parse_node_extensions called!");

    if (p_extensions.has("KHR_draco_mesh_compression")) {
        UtilityFunctions::print("Found KHR_draco_mesh_compression on node!");
        // Implement your logic here
        
        return OK;
    }

    // Otherwise, do not handle
    return ERR_SKIP;
}


Error GDDraco::_import_preflight(const Ref<GLTFState> &p_state, const PackedStringArray &p_extensions) {
    if (p_extensions.has("KHR_draco_mesh_compression")) {
        // Maybe set a flag in the GLTFState custom state
    }
    return OK;
}

PackedStringArray GDDraco::_get_supported_extensions() {
    PackedStringArray extensions;
    extensions.append("KHR_draco_mesh_compression");
    return extensions;
}
