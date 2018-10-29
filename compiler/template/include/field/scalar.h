@[ from "macro/integer.h" import integer_class ]

struct @(field.classname) : public @(integer_class(field)) {
    @(field.classname)(@(field.object.classname) & s, int index=0);
    @(field.type) & operator=(@(field.type) rhs);
} @(field.name);
