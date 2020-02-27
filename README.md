```C
/*
INTRODUCTION:
This is a fork of Dictu Programming Language, which is an implementation in C of
the Lox Programming Language.

    Lox: https://github.com/munificent/craftinginterpreters
  Dictu: https://github.com/Jason2605/Dictu

Dictu Lox is really a really lean Programming Language specification, but with some
quite common and established through time programming concepts.
And because of its simplicity, and its quite intuitive syntax and design, it is plain
easy to be productive in minutes. That is the basis of the interest.

As an extra, and because implements the syntax with such clarity and wisdom, is also
a joy to code, so it can be considered as a therapeutic tool in those full of complex
ways and days to program.

Of course development is at early stages, though even now is quite usefull and quite
attractive as is, but looks interesting to see how it is going to be evolved in time.

So, Many Thanks to Robert Nystrom first, also to Jason Hall, but and to all those got
involved in this fascinating idea.

My personal involvement is because, I was looking for a very small language that can
be embed in applications, but also and because the language is quite flexible and so
it instantly became attractive the idea to make the language to match my personal way
to express in code. This flexibility allows to be adopted by the intented environment
and also can adopt and use from the environment existing resources.

Note that because this code is at the early infancies, this safely can be considered
as an experiment and meant as demonstration only, for quite a long.
Probably and when Dictu will be stabilized, then perhaps this status might change one
day. Personally and for what I care though, is only about if current features work as
intented, and not at all for optimizations to the low level machine, as I do not even
have a clue of virtual machines.

The intentions for the syntax level, is to humanize it even more and to feel a little
bit like Lua. This will results to make written scripts to be incompatible for Dictu.
The intention though for the interpreter is to be able to execute Dictu scripts, with
the same semantics as Dictu. At least this is the idea, as really don't know if I can
catch up with Dictu development.

Another intention is to librarize it, so that it could be controlled by the host that
is using the machine. This means the possibility for more than one state and possible
changes to the function signatures.

Another thing is to end up in a mechanism that will use other source units (files or
strings), in a practical way. This also might change the semantics as the intention
here is that, the load mechanism should mean and an instance of a class.

It is also the intention to enhance or provide new native or optional classes, though
the intented target is for UNIX like systems, that mostly means Linux but also BSD's.

SYNTAX:
This is same as Dictu with the following enhancements:

   - 'not' is same as '!'

   - 'is' and 'isnot' are same as '==' and '!=' respectively

   - 'beg' and 'end' are same as '{' and '}' respectively

   - add the 'then' and 'orelse' keywords so that an if statement can be written as:

      if (not val) then
          statement ();
          ...
      orelse if (expression) then
          statement ();
          ...
      orelse then
          statement ();
          ...
      end

   - 'forever' is same as 'while (1)', and at the time of writing it really means
      forever, because 'break' hasn't been implemented yet, so there is no way to
      escape

The implementation is a bit of hack, but anyway this code is not focusing at the code
achievements, but to give the chance to the human being for code expressivity.

But because it does it, by modifying the evaluation string at the lexical anallysis
step (it modifies scanner.c), the code is rather fragile and should make assumptions.
So for this reason, the syntax should be strict and obey a way. Otherwise, yes, it is
an undefined behavior!

LICENSE:
This is same as Dictu, that is MIT.
*/
