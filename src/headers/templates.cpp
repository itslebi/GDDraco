#include "headers/templates.hpp"

namespace gddraco {
    int get_component_count(const char *acc_type) {
        if (strcmp(acc_type, "SCALAR") == 0) return 1;
        else if (strcmp(acc_type, "VEC2") == 0) return 2;
        else if (strcmp(acc_type, "VEC3") == 0) return 3;
        else if (strcmp(acc_type, "VEC4") == 0) return 4;
        else if (strcmp(acc_type, "MAT2") == 0) return 4;
        else if (strcmp(acc_type, "MAT3") == 0) return 9;
        else if (strcmp(acc_type, "MAT4") == 0) return 16;
        
        // default fallback (unknown type)
        return 1;
    }
}