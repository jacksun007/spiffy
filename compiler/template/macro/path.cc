@[ macro set_path(fs, from, ret) ]
@(fs.xrefname) * path = static_cast<@(fs.xrefname) *>(@(from));
if (path == nullptr) return @(ret);
@[ for xref in fs.xrefs ]
assert(path->@(xref.xref));
const @(xref.classname) & @(xref.xref) = *path->@(xref.xref);
(void)@(xref.xref);
@[ endfor ]
@[ endmacro ]

