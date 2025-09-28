/*
 * MIT License
 *
 * Copyright (c) 2025 itslebi, 3Onion
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

#ifndef CONVERT_HPP
#define CONVERT_HPP

#include <vector>
#include <cstdint>

namespace gddraco {

template <class MeshT>
inline void fill_indices_tris(const MeshT& mesh, std::vector<uint32_t>& indices) {
    const int faces = mesh.num_faces();
    indices.clear();
    indices.reserve(static_cast<size_t>(faces) * 3);
    for (int f = 0; f < faces; ++f) {
        const auto &tri = mesh.face(draco::FaceIndex(f));
        indices.push_back(static_cast<uint32_t>(tri[0].value()));
        indices.push_back(static_cast<uint32_t>(tri[1].value()));
        indices.push_back(static_cast<uint32_t>(tri[2].value()));
    }
}

}  // namespace gddraco

#endif //CONVERT_HPP
