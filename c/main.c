#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"
#include "util.h"

#include "linenoise.h"

#define VERSION "Dictu Version: 0.3.3\n"

static bool replCountBraces(char *line) {
    int leftBraces = 0;
    int rightBraces = 0;
    bool inString = false;

    for (int i = 0; line[i]; i++) {
        if (line[i] == '\'' || line[i] == '"') {
            inString = !inString;
        }

        if (inString) {
            continue;
        }

        if (line[i] == '{') {
            leftBraces++;
        } else if (line[i] == '}') {
            rightBraces++;
        }
    }

    return leftBraces == rightBraces;
}

static bool replCountQuotes(char *line) {
    int singleQuotes = 0;
    int doubleQuotes = 0;
    char quote = '\0';

    for (int i = 0; line[i]; i++) {
        if (line[i] == '\'' && quote != '"') {
            singleQuotes++;

            if (quote == '\0') {
                quote = '\'';
            }
        } else if (line[i] == '"' && quote != '\'') {
            doubleQuotes++;
            if (quote == '\0') {
                quote = '"';
            }
        } else if (line[i] == '\\') {
            line++;
        }
    }

    return singleQuotes % 2 == 0 && doubleQuotes % 2 == 0;
}

static void repl(VM *vm, int argc, const char *argv[]) {
    printf(VERSION);
    char *line;

    linenoiseHistoryLoad("history.txt");

    while((line = linenoise(">>> ")) != NULL) {
        char *fullLine = malloc(sizeof(char) * (strlen(line) + 1));
        snprintf(fullLine, strlen(line) + 1, "%s", line);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave("history.txt");

        while (!replCountBraces(fullLine) || !replCountQuotes(fullLine)) {
            free(line);
            line = linenoise("... ");

            if (line == NULL) {
                return;
            }

            char *temp = realloc(fullLine, strlen(fullLine) + strlen(line) + 1);

            if (temp == NULL) {
                printf("Unable to allocate memory\n");
                exit(71);
            }

            fullLine = temp;
            memcpy(fullLine + strlen(fullLine), line, strlen(line) + 1);

            linenoiseHistoryAdd(line);
            linenoiseHistorySave("history.txt");
        }

        interpret(vm, fullLine);

        free(line);
        free(fullLine);
    }
}

static void runFile(VM *vm, int argc, const char *argv[]) {
    char *source = readFile(argv[1]);
    InterpretResult result = interpret(vm, source);
    free(source); // [owner]

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}

#define EvalString(...) #__VA_ARGS__
char *__STDLIB__ = EvalString (
trait basename beg
   basename (name, delim) beg
     return name.split(delim).pop();
   end
end

class stdlib beg
  use basename;
end

class Path < stdlib beg
  init () beg
    this.delim = '/';
  end

  basename (name) beg
    return super.basename (name, this.delim);
  end

  dirname (name) beg
    var p = name.split(this.delim);
    var len = p.len();
    if (len is 1) then return "."; end
    return p[:len - 1].join(this.delim);
  end

  split (name) beg
    return name.split(this.delim);
  end
end
);

int main(int argc, const char *argv[]) {
    VM *vm = initVM(argc == 1, argc >= 2 ? argv[1] : "repl", argc, argv);
    interpret (vm, __STDLIB__);

    if (argc == 1) {
        repl(vm, argc, argv);
    } else if (argc >= 2) {
        runFile(vm, argc, argv);
    } else {
        fprintf(stderr, "Usage: dictu [path] [args]\n");
        exit(64);
    }

    freeVM(vm);
    return 0;
}
