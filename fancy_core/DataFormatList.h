#ifndef DECLARE_DATA_FORMAT
  #define DECLARE_DATA_FORMAT(...)
#endif

//                   Format,      ByteSize,  CopyableSizePlane0, CopyableSizePlane1,  NumComponents,   NumPlanes,   DepthStencil,   SRGB,    Compressed,    IsUintInt
DECLARE_DATA_FORMAT(NONE,             0,             0,                   0,                0,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(SRGB_8_A_8,       4,             4,                   0,                4,             1,           false,       true,       false,       0 )
DECLARE_DATA_FORMAT(RGBA_8,           4,             4,                   0,                4,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(BGRA_8,           4,             4,                   0,                4,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RG_8,             2,             2,                   0,                2,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(R_8,              1,             1,                   0,                1,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RGBA_16,          8,             8,                   0,                4,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RG_16,            4,             4,                   0,                2,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(R_16,             2,             2,                   0,                1,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RGB_11_11_10F,    4,             4,                   0,                3,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RGBA_16F,         8,             8,                   0,                4,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RGB_16F,          6,             6,                   0,                3,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RG_16F,           4,             4,                   0,                2,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(R_16F,            2,             2,                   0,                1,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RGBA_32F,        16,            16,                   0,                4,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RGB_32F,         12,            12,                   0,                3,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RG_32F,           8,             8,                   0,                2,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(R_32F,            4,             4,                   0,                1,             1,           false,       false,      false,       0 )
DECLARE_DATA_FORMAT(RGBA_32UI,       16,            16,                   0,                4,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RGB_32UI,        12,            12,                   0,                3,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RG_32UI,          8,             8,                   0,                2,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(R_32UI,           4,             4,                   0,                1,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RGBA_16UI,        8,             8,                   0,                4,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RGB_16UI,         6,             6,                   0,                3,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RG_16UI,          4,             4,                   0,                2,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(R_16UI,           2,             2,                   0,                1,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RGBA_8UI,         4,             4,                   0,                4,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RGB_8UI,          3,             3,                   0,                3,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RG_8UI,           2,             2,                   0,                2,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(R_8UI,            1,             1,                   0,                1,             1,           false,       false,      false,       1 )
DECLARE_DATA_FORMAT(RGBA_32I,        16,            16,                   0,                4,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(RGB_32I,         12,            12,                   0,                3,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(RG_32I,           8,             8,                   0,                2,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(R_32I,            4,             4,                   0,                1,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(RGBA_16I,         8,             8,                   0,                4,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(RGB_16I,          6,             6,                   0,                3,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(RG_16I,           4,             4,                   0,                2,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(R_16I,            2,             2,                   0,                1,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(RGBA_8I,          4,             4,                   0,                4,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(RGB_8I,           3,             3,                   0,                3,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(RG_8I,            2,             2,                   0,                2,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(R_8I,             1,             1,                   0,                1,             1,           false,       false,      false,       2 )
DECLARE_DATA_FORMAT(D_24UNORM_S_8UI,  4,             4,                   1,                2,             2,           true,        false,      false,       0 )  // TODO: NumComponents should probably be 1 here