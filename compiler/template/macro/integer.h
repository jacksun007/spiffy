@[ macro integer_class(f) ]
FS::Integer<@(f.type) @[ if f.is_big_endian() ], FS::TF_BIGENDIAN @[ endif ]>
@[ endmacro ]

@[ macro bitfield_class(f) ]
FS::Bitfield<@(f.type) @[ if f.is_big_endian() ], FS::TF_BIGENDIAN @[ endif ]>
@[ endmacro ]

