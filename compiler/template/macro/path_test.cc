@[ macro set_path(fs, from, ret, entity) ]

@(fs.xrefname) * path = static_cast<@(fs.xrefname) *>(@(from));
if (path == nullptr) return @(ret);
@[ for xref in fs.xrefs ]
@[ if xref.xref in entity.vars ]
assert(path->@(xref.xref));
const @(xref.classname) & @(xref.xref) = *path->@(xref.xref);
(void)@(xref.xref);
@[ endif ]
@[ endfor ]
@[ endmacro ]

