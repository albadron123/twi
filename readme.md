# This is my first interpreter, actually..

I've read this book called "crafting interpreters" by R. Nystrom and I wanted to do something by myself. I planned to craft a toy interpreter (lox) from the book, but I did dome minor syntax changes and did everything in the c++ from the very beginning (the book proposed to do java interpreter first and then do the c version).

I did not restrict myself with c-constructs and did some std::vectors and std::maps along the way. 

**IMPORTANT:** most of this code is actually garbage. I *DID NOT* intend to make an interpreter of any real language, I just mess around with the tools I've learnt and try to implement as much as I can myself. This imply that:
- I dont care about the corner cases that much *(eg: if the interpreter can jump between functions and execute code in them, that seems fine for me **EVEN** if i know that it does some wierd algorithm that doesn't alocate enough space for arbutrary long functions or that it creates an extra evironment for {} inside each function)*


**STATE:** i've done a procedural tree-walk interpreter for functions, expression and control-flow statements (it actually has some bugs). Then i really wanted to do some bytecode. Now I'm working on transferring everything into a bytecode representation.

**KINDA A ROADMAP:** 
*I think, that nobody will see this page anyway, so i will write sort-of-a-roadmap of the project here (not a real project, just some messing around).*
- Add appropriate error handling (some robustnest is good I think)
- Add static/dynamic arrays or any other indexed data structure
- Add types (in bytecode right now everything is just a number, I want to use the static memory and do something cool with it. RN i don't know whether I really want static or dynamic types. (Probably, will try to do bytecode on static types.)
- Add structs
- Add some analysis (and may be modules)
- Expand the default library
- Make both REPL & file compiling work correctly (probably add some flags)
- ???
- abandon the project and do something either more real or more esoteric

**SOME IDEAS (not nessesary will do them):**
- Correct debug-mode stack tracing utility

**DONE**

- Make functions in bytecode
- Write a fibonacci test on the bytecode implementation (+- 30 sec.)

