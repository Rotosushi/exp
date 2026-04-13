* sized integer support
* @import file support
* flow control (if/elif/else, while, for, switch)
* 


support global variables.

to support global names, we have to provide an evaluation context for the 
global variable initialization expression. This can be done with a lambda.
This lambda provides the evaluation context for the initialization expression,
and, as long as this expression resolves to a constant, the expression is 
compatible with our current code generation stack. 

The usage of a lambda for initialization expressions seems incompatible with the 
current way we bind symbols.

function definition uses context_enter_function

And I suppose a global variable will also use context_enter_function.
This will correctly generate a function body to hold the initialization expression,
however, we need somewhere to place the instructions which are generated from the 
initialization expression. We could define an "_init" implicit function in the runtime.
(similar to the "_start" function that the linker already expects) and place the 
initialization expressions for each global variable there.
