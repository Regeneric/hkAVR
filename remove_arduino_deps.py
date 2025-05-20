from SCons.Script import Import
Import("env")

env.Replace(
    SRC_FILTER=[
        "+<src/>",
        "+<lib/>"
    ]
)