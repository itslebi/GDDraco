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

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace gddraco {

// Error log always shown
inline void log_error(const godot::String &msg) {
    godot::UtilityFunctions::push_error("[GDDraco] " + msg);
}

// Warning log always shown
inline void log_warn(const godot::String &msg) {
    godot::UtilityFunctions::print("[GDDraco][warn] " + msg);
}

// Info log shown only in editor
inline void log_info(const godot::String &msg) {
    if (godot::Engine::get_singleton()->is_editor_hint()) {
        godot::UtilityFunctions::print("[GDDraco] " + msg);
    }
}

}  // namespace gddraco

#endif //LOGGING_HPP