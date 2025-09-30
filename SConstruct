if not ARGUMENTS.get("target"):
    ARGUMENTS["target"] = "template_debug"

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=[
    "src/",
    "include/draco/src",
    "include"
    ])

# --- BEGIN PATCHED BUILD FLAGS ---
try:
    if ARGUMENTS["target"] == "template_release":
        if env.get("platform") == "windows":
            env.Append(CCFLAGS=["/O2"])
        else: #Remove -flto if on WSL you get Permission denied error
            env.Append(CCFLAGS=["-O3", "-fno-exceptions", "-fno-rtti", "-flto"])
            env.Append(LINKFLAGS=["-s", "-flto"])
    env.Append(CCFLAGS=[
        "-DDRACO_POINT_CLOUD_COMPRESSION",
        "-DDRACO_MESH_COMPRESSION"
    ])
except Exception as e:
    print("GDDraco build flags warning:", e)
# --- END PATCHED BUILD FLAGS ---

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