This is a tool that generates Dictu sources, into a C single file unit, that allows to build dictu as a library.

https://github.com/dictu-lang/Dictu  

Optionally, it can enchnace Dictu syntax with the following enhancements:  

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

     (this syntax though not quite strict, it should obey these rules, for parsing reasons)   
  
   - 'do' as Lua starts a loop body  
  
   - 'forever' is same as 'while (true)'  

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
  #                     # Note: When this option is encountered, parsing argv stops
  #                     # and any subsequent argunent is treated as argument to this
  #                     # function, which after its execution the program exits
  #    --sysdir=`dir'   # system directory with write access, default [../sys]
  #    --builddir=`dir' # build directory, default [build/dictu] or [build/lai]
  #    --langcdir=`dir' # Dictu c sources directory, default [src/Dictu]
  #    --srcdir=`dir'   # source directory for this program, default [src]
  #    --donot-generate # do not generate any files
  #    --help, -h       # show this message


  # the generated files are installed into the build directory

  # The lai/dictu library is installed into the $(SYSDIR)/lib directory.
  # The lai/dictu sample interpreter is installed into the $(SYSDIR)/bin directory.
  # The lai.h/dictu.h header is installed into the $(SYSDIR)/include directory.

  # When translating lai scripts back to Dictu, the generated scripts are installed as the script
  # basename sans the extension name, plus the .du extension.

  # This builds without warnings with gcc-9.1.0, clang-9.0.0 and tcc last development.
```

LICENSE:  
It is the same as Dictu, that is MIT.  
