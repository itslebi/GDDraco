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

#ifndef VALIDATE_HPP
#define VALIDATE_HPP

#include <cstdint>
#include <vector>
#include <limits>

namespace gddraco {

// Validates if max_index is within the valid range of count
inline bool check_index_range(size_t count, uint32_t max_index) {
    return (count <= std::numeric_limits<uint32_t>::max()) && (max_index < count);
}

// Simple equality check between expected and actual component counts
inline bool expect_components(int got, int expected) {
    return got == expected;
}

}  // namespace gddraco

#endif //VALIDATE_HPP
