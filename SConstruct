if not ARGUMENTS.get("target"):
    ARGUMENTS["target"] = "template_debug"

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=[
    "src/",
    "include/draco/src",
    "include"
    ])

sources = (
    # GDDraco Source
    Glob("src/*cpp") +
    # Godot CPP Source
    Glob("include/src/*.cpp") +
    # Draco SDK Source
    Glob("include/draco/src/draco/animation/*.cc") +
    Glob("include/draco/src/draco/attributes/*.cc") +
    Glob("include/draco/src/draco/mesh/*.cc") +
    Glob("include/draco/src/draco/core/*.cc") +
    Glob("include/draco/src/draco/compression/*.cc") +
    Glob("include/draco/src/draco/metadata/*.cc") +
    Glob("include/draco/src/draco/point_cloud/*.cc") +
    Glob("include/draco/src/draco/compression/attributes/*.cc") +
    Glob("include/draco/src/draco/compression/attributes/prediction_schemes/*.cc") +
    Glob("include/draco/src/draco/compression/bit_coders/*.cc") +
    Glob("include/draco/src/draco/compression/entropy/*.cc") +
    Glob("include/draco/src/draco/compression/mesh/*.cc") +
    Glob("include/draco/src/draco/compression/point_cloud/*.cc") +
    Glob("include/draco/src/draco/compression/point_cloud/algorithms/*.cc")
)

library = env.SharedLibrary("demo/bin/GDDraco{}{}".format(env["suffix"], env["SHLIBSUFFIX"]), source = sources)

Default(library)