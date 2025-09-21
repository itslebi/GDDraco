#ifndef FILE_HPP
#define FILE_HPP

#include <godot_cpp/classes/sprite2d.hpp>
#include <godot_cpp/variant/vector2.hpp>

namespace godot {
    class BoucySprite: public Sprite2D {
        GDCLASS(BoucySprite,Sprite2D);

        public:
            double left_bounds;
            double right_bounds;
            double top_bounds;
            double bottom_bounds;
            Vector2 velocity;
            double speed = 350;

        protected:
            static void _bind_methods();
        
        public:
            BoucySprite();
            ~BoucySprite();

            void _process(double delta) override;
    };
}

#endif //FILE_HPP