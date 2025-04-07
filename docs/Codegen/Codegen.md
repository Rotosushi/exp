
## Strategy
We use a variant of a classical strategy for code generation. The "Macro Expansion" technique. Whereby a given [[bytecode]] instruction is expanded into one or more target 
assembly instructions. We have explicitly programmed the expansion, and one technique for making the compiler support multiple targets would be to generate the expansions programmatically. However, given the historically known limitations of this technique, it seems pointless to put in all of that work for something so limited. I have no 