
# TODO

- refactor exp to process an assembly like lang.

## refactor exp to process an assembly like lang

### Project Restructuring

- [] each subcomponent of the compiler needs it's own library.
     This should ease maintinence of the project as a whole,
     as it will make visible the API surface of each component.
     And properly managing the API surface will be what increases
     the modularity of the system, reduce maintinence headaches
     and allow new components to be developed and plugged in to the
     existing system.
  - [] core: holds the program entry point. and any system specific code
       that needs to be there to support program startup, and cleanup.
  - [] support: holds basic program utilities and platform specific
       abstractions. This is the library which is used to abstract
       the implementation of the compiler away from the differences
       between C on Linux, Windows, macOS, BSD, etc...
       It defines, String, Assert, Breakpoint, Process, Message, etc
  - [] context: holds the context specific information to the current
       run of the compiler. This is where the incoming arguments to
       the program are parsed, where the options controlling the
       compilation process are decided, and where the process of compilation
       as a whole is orchestrated. It defines Context, ProgramOptions,
       StringInterner, TypeInterner.
  - [] scanning: holds the frontend tokenization and parsing logic
       which is used to construct the TIIR. It defines, Token, Lexer,
       Parser,
  - [] TIIR: holds the target independent intermediate representation.
       This is the library which defines the API surface of the
       representation, the different passes of the compiler will
       interact via this API, instead of knowing the details of the
       TIIR directly. if the API is well-designed, this is possible.
       Theoretically we should be able to swap out the underlying
       memory representation of the TIIR and have the rest of the
       compiler still work.
       I am conceptualizing it as defining Instruction, Operand, Function,
       Module,
  - [] passes: holds the API of the various passes over the TIIR, which
       either verify, mutate, or lower the TIIR to the TSIR. It is the
       boundary between the the TIIR and the TSIR, as well as the
       implementation of the Code Generator.
       I am conceptualizing it as defining objects such as PassManager,
       VerifyModulePass, DominatorPass, RegisterAllocationPass,
       StrengthReductionPass, FunctionInliningPass, LoopUnrollingPass,
       SerializePass, etc.
  - [] TSIR: holds the target specific intermediate representation.
       This is the library which defined the API surface of the
       TSIR, and abstracts the different passes of the compiler away
       from the details of which particular target architecture is
       being lowered to. It is simultaneously the API boundary of the
       Target Architecture libraries. Such that a new target architecture
       may be supported by providing the implementation details of the
       API, and in this way new targets may be added to the compiler.
       and these new targets can reuse the same optimizations available.
       I am conceptualizing it as defining objects such as:
       TargetInstruction, TargetFunction, TargetObjectFile, TargetDebugInformation,
       etc,

- [] We don't provide a runtime library ourselves. That is the job of the language
     implemented on top of exp. This program is more akin to an assembler that just
     creates the object files to then subsequently be linked together.
     This is a major overhaul of the direction of the project.

### Scanning

- [] define a new grammar for this assembly like langauge
- [] rewrite the token set to the new set needed by the new grammar
- [] rewrite the lexer in re2c to begin support for utf8
- [] rewrite the parser to the new grammar
  - [] The parser uses a new InstructionBuilder Struct to construct instructions
       We can use the implementation of InstructionBuilder to encapsulate trivial
       constant folding, and reduce the complexity of the Context

### IMR -> TIIR

- [] rename IMR to TIIR project wide.
- [] Instructions need to be reworked to support the new grammar.
  - [] There structure of Opcode kind + 3 variant Operands forces our
       hand into having 3 nested switch statements to support the
       generation of code for each form of the instruction.
       it is my opinion that a single large switch statement will be
       faster, and less of a maintinence headache. We will be moving
       towards encoding the layout of the instruction into the Opcode
       of the instruction. so once you select over the opcode you
       immediately know which variant each of the operands to the
       instruction are, and can begin generating code.
- [] rewrite Tuples, they need to be held per function, instead of globally.
- [] The context and the current TranslationUnit are currently the same structure.
     This might be too much responsibility for a single object. We need to
     define a TranslationUnit object and have it model only the responsibilities
     of the TU specifically. The Context can manage the responsibilities which
     are more general than a particular TU, such as which target we are lowering to,
     and the storage of the current error set and so on.
- [] Locals currently store their lifetime, is that pass-specific knowledge
     or is that knowledge that each pass will use?
- [] Design a PassManager which allows the definition and sequencing of distinct
     passes over the IR. Currently, the set and sequence of passes is hard-coded
     into the definition of the "compile" function. We can create a structure
     which allows us to register a set and sequence of passes, and then run that
     sequence in order to perform all analysis, optimization, code generation, and
     serialization work.

### Intrinsics

- [] compiler intrinsics are specific functionality that is built into the compiler
     and not provided by the standard library to user code. they are meant to be
     the necessary bits of plumbing to build a standard library on top of.
     I am not satisfied with their implementation as a free-function which always
     takes a Context pointer. Though I am not sure where to take them either.
- [] align_of and size_of are target dependent functionality. we need to decide on
     a target specific API boundary that we can interface with, so the functions are
     available, and provided by a backed that can be selected over.
- [] type_of is an intrinsic in a higher level language, I wanted to provide it for a
     theoretical typeof(...) expression in said higher level language. As our target
     is no longer a higher level language, is the intrinsic even needed?

### Support

- [] rewrite error handling
  - [] errors need to be held in a list in the context.
  - [] we need a new ErrorBuilder structure to construct Errors, this will reduce
       the complexity of the Context struct.
  - [] we don't want to panic on stdlib errors, we want to produce an error with the
       ErrorBuilding, and Log it to the Context.

## Cleanup

- [] support/scalar.h is an abstruse name, i prefer its original name "support/int_types.h"
- [] support/string.h is taking over the responsibilities of a support/path.h structure.
     lets move path related functionality into a separate file. it can still use a string
     as an implementation detail.
