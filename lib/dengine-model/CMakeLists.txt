set(dengine-model-src
src/model.c
)

set(dengine-model-hdr
include/dengine-model/model.h
)
set(dengine-model-link
    dengine-core dengine-utils)
dengine_add_lib(dengine-model "3D Models")
