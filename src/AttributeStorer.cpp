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

#include "AttributeStorer.hpp"

AttributeStorer::AttributeStorer(godot::String name, godot::GLTFAccessor::GLTFComponentType comp_type, 
        godot::GLTFAccessor::GLTFAccessorType acc_type, int draco_ID, int count, bool normalized)
    : name(name), comp_type(comp_type), acc_type(acc_type), draco_ID(draco_ID), count(count), normalized(normalized) {}

AttributeStorer::AttributeStorer() {}

int AttributeStorer::get_comp_type() const {
    return static_cast<int>(comp_type);
}

void AttributeStorer::set_comp_type(godot::GLTFAccessor::GLTFComponentType comp_type) {
    this->comp_type = comp_type;
}

char* AttributeStorer::get_acc_type() const {
    switch (acc_type) {
        case godot::GLTFAccessor::TYPE_SCALAR:
            return "SCALAR";
        case godot::GLTFAccessor::TYPE_VEC2:
            return "VEC2";
        case godot::GLTFAccessor::TYPE_VEC3:
            return "VEC3";
        case godot::GLTFAccessor::TYPE_VEC4:
            return "VEC4";
        case godot::GLTFAccessor::TYPE_MAT2:
            return "MAT2";
        case godot::GLTFAccessor::TYPE_MAT3:
            return "MAT3";
        case godot::GLTFAccessor::TYPE_MAT4:
            return "MAT4";
        default:
            return "UNKNOWN";
    }
}

void AttributeStorer::set_acc_type(godot::GLTFAccessor::GLTFAccessorType acc_type) {
    this->acc_type = acc_type;
}