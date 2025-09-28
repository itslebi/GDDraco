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

#ifndef GDDRACO_CONVERT_HPP
#define GDDRACO_CONVERT_HPP

#include <cstdint>
#include "draco/mesh/mesh.h"

namespace gddraco {

    // Places the indices from tris to the correct OutputIterator
    template <class MeshT, class OutputIt>
    inline void fill_indices_tris(const MeshT& mesh, OutputIt out) {
        const int faces = mesh.num_faces();
        for (int f = 0; f < faces; ++f) {
            const auto& tri = mesh.face(draco::FaceIndex(f));
            *out++ = tri[0].value();
            *out++ = tri[1].value();
            *out++ = tri[2].value();
        }
    }

}  // namespace gddraco

#endif //GDDRACO_CONVERT_HPP
