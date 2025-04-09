
# Language Specification

* The type system of the compiler is a standard static type system.
	* We don't currently support user defined types, other than functions and tuples.
	* We don't currently support polymorphism. (Though strictly speaking tuples are a polymorphic type.)
* we only support local constants, defined with the "const" keyword
* we only support a single integer type "i64".
* we only support the five basic arithmetic operations "+ - * / %"
* we only support two control flow structures: call and return
* we support functions, with up to 255 arguments and 1 return value
* we support tuples with up to 2^32 elements
* we do not support assignment currently, it is planned to be supported
* we do not support global constants, these are planned. Global Variables are as well though these are only as strictly necessary.

# Planned Support

* Integral and Real types
	* i8/16/32/64
	* u8/16/32/64
	* f32/64

* Vector Integral and Real types
	* i8/16/32/64_vec_2/3/4
	* u8/16/32/64_vec_2/3/4
	* f/d_vec_2/3/4
