```C
/* This is a dialect of the Programming Language Lox, that it can be used as a
   runtime interpreter. It is an academic project and mostly a chance to study
   some of the mechanics of the Programming Languages.

So, this is actually a fork of Dictu Programming Language, which is an implementation
in C of the Lox Programming Language and which is described in a book, see at:

    Lox: https://github.com/munificent/craftinginterpreters
  Dictu: https://github.com/Jason2605/Dictu

Dictu Lox is really a really lean Programming Language specification, but with some
quite common and established through time programming concepts.
And because of its simplicity, and its quite intuitive syntax and design, it is plain
easy to learn and be productive in minutes. And that is the basis of the interest.

As an extra, and because implements the syntax with such clarity and wisdom, is also
a joy to code, so it can be considered as a therapeutic tool in those full of complex
ways and days to program.

Of course development is at early stages, though even now is quite usefull and quite
attractive as is, but looks interesting to see how it is going to be evolved in time.

So, Many Thanks to Robert Nystrom and also to Jason Hall, but and to all of those who
got involved in this fascinating idea.

My personal involvement is because, I was looking for a very small language that can
be embed in applications. But and because the language is so flexible and so simple,
it instantly became attractive the idea to make the language to match my personal way
to express in code and at the same time to gain some little knowledge on the road.

Note that because this code is at the early infancies, this safely can be considered
as an experiment and meant as demonstration only, for quite a long.
Probably and when Dictu will be stabilized, then perhaps this status might change one
day. Personally and for what I care though, is only about if current features work as
intented, and not at all for optimizations to the low level machine, as I do not even
have a clue of virtual machines. So the real interest is on the semantics and syntax.

The intentions for the syntax level, is to humanize it even more and to feel a little
bit like Lua.
The intention for this interpreter is to be able to execute Dictu scripts, with the
same semantics as Dictu, so Dictu scripts should be run and without a single change
by this interpreter.

Another intention is to librarize it, so that it could be controlled by the host that
is using the machine. This means the possibility for more than one state¹ and possible
changes to the function signatures or probably new functions

 ¹[note: that recent upstream development that removed any references to global scope
         variables, assists with a generous way to achieve this goal]

It is also the intention to enhance or provide new native or optional classes, though
the intented target is UNIX like system.

Syntax:
This is same as Dictu with the following enhancements:

   - 'not' is same as '!'

   - 'is' and 'isnot' are same as '==' and '!=' respectively

   - 'beg' and 'end' are same as '{' and '}' respectively

   - add the 'then' and 'orelse' keywords so that an if statement can be written as:

      if (not expression) then
          statement ();
          ...
      orelse if (expression) then
          statement ();
          ...
      orelse then
          statement ();
          ...
      end

     (this syntax though not quite strict, it should obey these rules, if for no
      other reason (though there are reasons), then just for clarity and consistency)

   - 'do' as Lua starts a loop body

   - 'forever' is same as 'while (true)'

The implementation is a bit of hack, but anyway this code is not focusing at the code
achievements, but to give the chance to the human being for code expressivity.

But because it does it, by modifying the evaluation string at the lexical anallysis
step (it modifies scanner.c), the code is rather fragile and should make assumptions.
So for this reason, the syntax should be strict and obey a way. Otherwise, yes, it is
an undefined behavior!

Also as extensions to Dictu (see at Dictu docs sources for Dictu semantics):

  - __FILE__ refers to the path of the compilation unit. The value has to be stored
    in a top variable in the compilation unit and with a unique name, otherwise it
    is undefined behabior.
    Note: this merged upstream as __file__.

  - provide the len() method to the String, List, Dict, Set DataTypes and also other
    builtin functions.
    Note: this merged upstream, plus toNumber(), toString(), toBool()

  - import can accept also expressions (Dictu 'import' accepts only strings)
    Note: since this is under upstream development, import at this stage, can not
    accept expressions anymore, untill is resolved

Build time deferences:

  - make DISABLE_HTTP=1 avoids to link against libcurl, which Dictu build system does
    it by default as a dependency to the HTTP class. In that case to disable also the
    http tests, use "--disable-http" when running the tests.
    Note: this is merged upstream.

*/

/* Update (end of May of 2020):

   Since almost all of the development wes merged upstream, plus there is an extremely
   nice collaboration, there was no really need to continue with the fork in a lonely
   trip.

   Also, because there is the will to execute Dictu scripts without any modification
   I had always to catch up with Dictu development.

   So this ended up as a more general tool with some broader functionality, and which
   also allowed to build Dictu as a library and particularly as a single C unit, which
   is very convenient when you are working with C.

   This tool works by parsing the unmodified Dictu sources and generates the required
   files, that can be compiled as a library.

   It can also parses a lai script and tranlate it as a Dictu script.
*/

```
Usage:
```sh
  # clone/update Dictu sources (note that this is not strictly required if you
  # already have Dictu sources in another directory in the filesystem)

  make clone-upstream
  make update-upstream

  # build the executable named lmake
  make

  # generate the required files
  ./lmake

  # options:
  #   --enable-http     # enable the http module (requires libcurl)
  #   --enable-repl     # enable an interactive session when building the interpreter
  #   --build-library   # invokes make to build the library
  #   --build-interp    # invokes make to build the sample interpreter
  #   --clean-installed # invokes make clean to clean installed generated objects
  #   --clean-build     # removes generated files from the build directory
  #   --enable-lai      # enable lai dialect (see above for the syntax extensions to Dictu)
  #   --parse-lai       # parse lai script and output a Dictu script with a .du extension
  #                      # Note: When this option is encountered, parsing argv stops
  #                      # and any subsequent argunent is treated as argument to this
  #                      # function, which after its execution the program exits
  #    --sysdir=`dir'   # system directory with write access, default [../sys]
  #    --builddir=`dir' # build directory, default [build/dictu] or [build/lai]
  #    --langcdir=`dir' # Dictu c sources directory, default [src/Dictu]
  #    --srcdir=`dir'   # source directory for this program, default [src]
  #    --donot-generate # do not generate any files
  #    --help, -h       # show this message

  # The lai/dictu library is installed into the $(SYSDIR)/lib directory.
  # The lai/dictu sample interpreter is installed into the $(SYSDIR)/bin directory.
  # The lai.h/dictu.h header is installed into the $(SYSDIR)/include directory.

  # When parsing lai scripts to Dictu, the generated scripts installed as the script
  # basename sans the extension name, plus the .du extension.

  # This builds without warnings with gcc-9.1.0, clang-9.0.0 and tcc last development.
```

```C
LICENSE:
It is the same as Dictu, that is MIT.
```
