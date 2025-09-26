#ifndef PRIMITIVE_DATA_HPP
#define PRIMITIVE_DATA_HPP

#include <godot_cpp/classes/array_mesh.hpp>

//Helper class to join important related primitive data together
class PrimitiveData {
    public:
        int material_Idx;
        godot::Ref<godot::ArrayMesh> primitive;

        PrimitiveData(int material_Idx, godot::Ref<godot::ArrayMesh> primitive);
};

#endif //PRIMITIVE_DATA_HPP