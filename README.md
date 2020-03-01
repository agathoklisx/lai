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
bit like Lua. This it will result to make written scripts to be incompatible for the
Dictu interpreter.
The intention for this interpreter is to be able to execute Dictu scripts, with the
same semantics as Dictu, so Dictu scripts should be run and without a single change
by this interpreter. At least this is the idea, as really don't know if I can catch
up with Dictu development. But for the moment it passes all the tests from upstream
that are located under the tests directory and should be updated.

Another intention is to librarize it, so that it could be controlled by the host that
is using the machine. This means the possibility for more than one state and possible
changes to the function signatures or probably new functions.

Another thing is to end up in a mechanism that will use other source units (files or
strings), in a practical way. This also might change the semantics, as the intention
here is that, the load mechanism should mean and an instance of a class, so that can
be stored in a variable.

It is also the intention to enhance or provide new native or optional classes, though
the intented target is for UNIX like systems, that mostly means Linux but also BSD's.

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

   - 'forever' is same as 'while (1)', and at the time of writing it really means
      forever, because 'break' hasn't been implemented yet, so there is no way to
      escape

The implementation is a bit of hack, but anyway this code is not focusing at the code
achievements, but to give the chance to the human being for code expressivity.

But because it does it, by modifying the evaluation string at the lexical anallysis
step (it modifies scanner.c), the code is rather fragile and should make assumptions.
So for this reason, the syntax should be strict and obey a way. Otherwise, yes, it is
an undefined behavior!

Also as extensions to Dictu (see docs/docs/* for Dictu semantics):

  - __FILE__ refers to the path of the compilation unit. The value has to be stored
    in a top variable in the compilation unit and with a unique name, otherwise it
    is undefined behabior

  - provide the len() method to the String, List, Dict, Set DataTypes (Dictu exposes
    len() as native, but we want to be consistent, though there are a couple more that
    has to be implemented, mainly the type(), to_s()  and to_i() methods

  - import can accept also expressions (Dictu 'import' accepts only strings)
*/
```
Usage:
```sh
  # make and check the futures
  make && ./lai tests/lai/syntax.lai
```
```C
/*
This builds without warnings with gcc-9.1.0, clang-9.0.0 and tcc last development.

The above test script should include the whole syntax of this dialect.

But here is some funny and valid code, and with a personal conclusion at the end.
  note: why anyone want to do that? but is possible
*/
```ruby
class int beg
  init (nr) beg
    this.val = nr;
  end

  to_s () beg
    return str(this.val);
  end

  plus (nr) beg
    this.val += nr;
    return this;
  end

  __ () beg
    return this.val;
  end
end

var n = int (10);
assert (n.plus(10).plus(10).__() == 30);
assert (n.plus(10).to_s() == "40");
```

/* okey, this is an extreme but those things happens actually under the hood in
   a programming language that abstract details to offer convienences, so there
   is a price. For a scripting language which is being used as a driver this can
   be acceptable though. I guess everyone is willing to pay the price but also is
   logical to expect to get at least some joy. And it would be perfect if anyone
   could adjust the language to a self way, instead of fighting to do the opposite.

   The hard part here is that usually in those cases you are out of lack, if you
   want at the same time to use an existing pool of this particular environment
   and pick up shared functionality, and which you might want to do it by yourself.

   I guess this lonely journey might never reward you with something practical in
   human established terms (which is simply means money) or in the good case glory.
   But it usually rewards you with knowledge or fun, and that is what happened in
   this case.

   Again as a guess, the most fascinating thing here is that you do not have to, and
   for another time again to learn just another way to express, as you already know
   the patterns.

   I really do not know why i believe that, if it could attach an interest and this
   interest could help with or|and improve some details, it can have a really nice
   future.

   So i believe that this little jewel, can become an excellent tool for introducing
   humans to the most basic programming concepts, in just a couple of days, and also
   can become a perfect language to non programmers, and finally and as plus a relief
   from the tyrrany of the complexity of the programming language environment, to the
   poor programmers minds, when what they really want is write in a standard expected
   and with the simplest possible way (kinda like POSIX sh but without the baggage of
   the past).

   And they do not want to use a usually huge PL, which it usually brings also and a
   dependency to an ecosystem, and for to handle this ecosystem properly it needs a
   dependency to an abstraction tool, where the abstraction tool can have also have a
   dependency to even in a PL other than the one that is the abstraction tool... so...

   Why we have to fight with a complexity and why we have to add bloat with no reason
   when we can do all the things by prioritizing simplicity?
   After those years and with so many superb coders, we should be able by now to feel 
   the absolute harmony of a synthetic code, that can offer safety, flexibility, and
   an own way to express. A way that can be understandable and parsable. A way that
   it will be based on intentions and how to make them obvious. It's the intentions
   that really matters, imho, when you develop own code or when trying to assist to
   another codebase.

   And finally with a syntax that can be learned in a few minutes. And another half
   an hour to learn the standard library. An another couple of hours to understand
   the underlying code of the language and to be able to extend it.

   This is the case with Lox. Plus and because is designed around so many very very
   common patterns and techniqes, I could bet that the code, it could be translated
   to other languages into a night.

   This conclusion written at the first day of Spring of 2020, at 02 o'clock at the
   midnight and after half of a week of a probably overzealous messing with the code,
   but and one of the most interesting coding experiences.
 */

/*
LICENSE: Oh i remembered at the very end. Okey we SHOULD have licenses! Hmm, let me
think...
Normally it should be the same as Dictu, that is MIT, but i wonder if it could also
be in Public Domain, as this is mostly a research project and i really like to think
in terms of public contributions, as we are also participating in this big party of
this unbelievable huge exchange to this endless path of evolution, and which can also
bring satisfaction as a gift.

Anyway the Dictu LICENSE is attached to this project.

Again thanks for this journey, I feel that owe a lot.
*/
