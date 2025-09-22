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

    // You can check for a custom extension key, for example:
    if (p_extensions.has("KHR_draco_mesh_compression")) {
        UtilityFunctions::print("Found KHR_draco_mesh_compression on node!");
        // Implement your logic here
        return OK;
    }

    // Otherwise, do not handle
    return ERR_SKIP;
}
