# cmake.py
import os
import glob
from SCons.Script import DefaultEnvironment, Execute

env = DefaultEnvironment()

# Only run the CMake steps once per invocation
if not env.get("CMAKE_DONE", False):
    env["CMAKE_DONE"] = True

    # 1) Determine paths
    project_dir = env['PROJECT_DIR']            # your repo root
    build_dir   = os.path.join(project_dir, "cmake-build")

    # 2) Create out-of-source build dir
    os.makedirs(build_dir, exist_ok=True)

    # 3) Configure & build via CMake
    cmake = env.Detect("cmake") or "cmake"
    # -S = source dir, -B = build dir
    Execute(f"{cmake} -S {project_dir} -B {build_dir}")
    # build with parallel jobs
    Execute(f"{cmake} --build {build_dir} -- -j{os.cpu_count()}")

    # 4) Find the ELF (expects one .elf in cmake-build/)
    elf_list = glob.glob(os.path.join(build_dir, "*.elf"))
    if not elf_list:
        env.Exit("ERROR: No .elf found in cmake-build/")
    elf_path = elf_list[0]

    # 5) Tell PIO to upload & debug that ELF via picoprobe
    env.Replace(
        PROGNAME       = os.path.splitext(os.path.basename(elf_path))[0],
        PROGSUFFIX     = os.path.splitext(elf_path)[1],
        PROGRAM_BUILDER = lambda source, target, env: elf_path
    )
