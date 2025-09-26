#include "PrimitiveData.hpp"

PrimitiveData::PrimitiveData(int material_Idx, godot::Ref<godot::ArrayMesh> primitive)
    : material_Idx(material_Idx), primitive(primitive) {}
