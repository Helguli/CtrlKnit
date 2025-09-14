import os
Import("env")

# include toolchain paths (i.e. to have stuff like the Arduino framework headers present in the compile commands)
env.Replace(COMPILATIONDB_INCLUDE_TOOLCHAIN=True)

# override compilation DB path
env.Replace(COMPILATIONDB_PATH=os.path.join("$PROJECT_DIR", "compile_commands.json"))
