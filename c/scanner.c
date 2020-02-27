#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    const char *start;
    const char *current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char *source) {
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

static char advance() {
    scanner.current++;
    return scanner.current[-1];
}

static char peek() {
    return *scanner.current;
}

static char peekNext() {
    if (isAtEnd()) return '\0';
    return scanner.current[1];
}

static bool match(char expected) {
    if (isAtEnd()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;
}

static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    token.length = (int) (scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static Token errorToken(const char *message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int) strlen(message);
    token.line = scanner.line;

    return token;
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;

            case '\n':
                scanner.line++;
                advance();
                break;

            case '/':
                if (peekNext() == '*') {
                    // Multiline comments
                    advance();
                    advance();
                    while (true) {
                        while (peek() != '*' && !isAtEnd()) {
                            if (advance() == '\n') {
                                scanner.line++;
                            }
                        }

                        if (peekNext() == '/') {
                            break;
                        }
                        advance();
                    }
                    advance();
                    advance();
                } else if (peekNext() == '/') {
                    // A comment goes until the end of the line.
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;

            default:
                return;
        }
    }
}

static TokenType checkKeyword(int start, int length,
                              const char *rest, TokenType type) {
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
    switch (scanner.start[0]) {
        case 'a':
            return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'b':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'e':
                        return checkKeyword(2, 1, "g", TOKEN_LEFT_BRACE);
                    case 'r':
                        return checkKeyword(2, 3, "eak", TOKEN_BREAK);
                 }
             }
             break;
        case 'c':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'l':
                        return checkKeyword(2, 3, "ass", TOKEN_CLASS);
                    case 'o':
                        return checkKeyword(2, 6, "ntinue", TOKEN_CONTINUE);
                }
            }
            break;
        case 'd':
            return checkKeyword(1, 2, "ef", TOKEN_DEF);
        case 'e':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'n':
                        return checkKeyword(2, 1, "d", TOKEN_RIGHT_BRACE);
                    case 'l':
                        return checkKeyword(2, 2, "se", TOKEN_ELSE);
                }
            }
            break;
        case 'f':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a':
                        return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o':
                        if (scanner.current - scanner.start == 3)
                            return checkKeyword(2, 1, "r", TOKEN_FOR);

                        if (TOKEN_IDENTIFIER == checkKeyword(2, 5, "rever", 0))
                            return TOKEN_IDENTIFIER;

                        char *modified = (char *) scanner.start;
                        char replaced[] = "while (1) {";
                        for (int i = 0; i < 11; i++)
                            modified[i] = replaced[i];
                        scanner.start   = (const char *) modified;
                        scanner.current = scanner.start + 5;
                        return TOKEN_WHILE;
                }
            }
            break;
        case 'i':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'f':
                        return checkKeyword(2, 0, "", TOKEN_IF);
                    case 'm':
                        return checkKeyword(2, 4, "port", TOKEN_IMPORT);
                    case 's':
                      if (scanner.current - (scanner.start + 1) > 1)
                          return checkKeyword(2, 3, "not", TOKEN_BANG_EQUAL);
                      return checkKeyword(1, 1, "s", TOKEN_EQUAL_EQUAL);
                 }
            }
            break;
        case 'n':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'o':
                        return checkKeyword(2, 1, "t", TOKEN_BANG);
                    case 'i':
                        return checkKeyword(2, 1, "l", TOKEN_NIL);
                }
            }
            break;
        case 'o':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'r':
                        if (scanner.current - (scanner.start + 1) > 1) {
                            if (TOKEN_ELSE != checkKeyword(2, 4, "else", TOKEN_ELSE))
                                return TOKEN_IDENTIFIER;

                            char *modified = (char *) scanner.start;
                            modified[0] = '}'; modified[1] = ' ';
                            scanner.start   = (const char *) modified;
                            scanner.current = scanner.start + 1;
                            return TOKEN_RIGHT_BRACE;
                        }
                    return checkKeyword(1, 1, "r", TOKEN_OR);
                }
             }
             break;
        case 'r':
            return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'u':
                        return checkKeyword(2, 3, "per", TOKEN_SUPER);
                    case 't':
                        return checkKeyword(2, 4, "atic", TOKEN_STATIC);
                }
            }
            break;
        case 't':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h':
                  	  if (scanner.current - scanner.start > 1) {
                            switch (scanner.start[2]) {
                                case 'e':
                                    return checkKeyword(3, 1, "n", TOKEN_LEFT_BRACE);
                                case 'i':
                                    return checkKeyword(3, 1, "s", TOKEN_THIS);
                            }
                        }
                        break;
                    case 'r':
                        if (scanner.current - scanner.start > 1) {
                            switch (scanner.start[2]) {
                                case 'u':
                                    return checkKeyword(3, 1, "e", TOKEN_TRUE);
                                case 'a':
                                    return checkKeyword(3, 2, "it", TOKEN_TRAIT);
                            }
                        }
                    break;
                }
            }
            break;
        case 'u':
            return checkKeyword(1, 2, "se", TOKEN_USE);
        case 'v':
            return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w':
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h':
                        return checkKeyword(2, 3, "ile", TOKEN_WHILE);
                    case 'i':
                        return checkKeyword(2, 2, "th", TOKEN_WITH);
                }
            }
            break;
    }

    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    while (isAlpha(peek()) || isDigit(peek())) advance();

    return makeToken(identifierType());
}

static Token number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the "."
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}


static Token string(char stringToken) {
    while (peek() != stringToken && !isAtEnd()) {
        if (peek() == '\n') {
            scanner.line++;
        } else if (peek() == '\\') {
            scanner.current++;
        }
        advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    // The closing " or '.
    advance();
    return makeToken(TOKEN_STRING);
}

void backTrack() {
    scanner.current--;
}

Token scanToken() {
    skipWhitespace();

    scanner.start = scanner.current;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

    switch (c) {
        case '(':
            return makeToken(TOKEN_LEFT_PAREN);
        case ')':
            return makeToken(TOKEN_RIGHT_PAREN);
        case '{':
            return makeToken(TOKEN_LEFT_BRACE);
        case '}':
            return makeToken(TOKEN_RIGHT_BRACE);
        case '[':
            return makeToken(TOKEN_LEFT_BRACKET);
        case ']':
            return makeToken(TOKEN_RIGHT_BRACKET);
        case ';':
            return makeToken(TOKEN_SEMICOLON);
        case ':':
            return makeToken(TOKEN_COLON);
        case ',':
            return makeToken(TOKEN_COMMA);
        case '.':
            return makeToken(TOKEN_DOT);
        case '/': {
            if (match('=')) {
                return makeToken(TOKEN_DIVIDE_EQUALS);
            } else {
                return makeToken(TOKEN_SLASH);
            }
        }
        case '*': {
            if (match('=')) {
                return makeToken(TOKEN_MULTIPLY_EQUALS);
            } else if (match('*')) {
                return makeToken(TOKEN_STAR_STAR);
            } else {
                return makeToken(TOKEN_STAR);
            }
        }
        case '%':
            return makeToken(TOKEN_PERCENT);
        case '-': {
            if (match('-')) {
                return makeToken(TOKEN_MINUS_MINUS);
            } else if (match('=')) {
                return makeToken(TOKEN_MINUS_EQUALS);
            } else {
                return makeToken(TOKEN_MINUS);
            }
        }
        case '+': {
            if (match('+')) {
                return makeToken(TOKEN_PLUS_PLUS);
            } else if (match('=')) {
                return makeToken(TOKEN_PLUS_EQUALS);
            } else {
                return makeToken(TOKEN_PLUS);
            }
        }
        case '&':
            return makeToken(match('=') ? TOKEN_AMPERSAND_EQUALS : TOKEN_AMPERSAND);
        case '^':
            return makeToken(match('=') ? TOKEN_CARET_EQUALS : TOKEN_CARET);
        case '|':
            return makeToken(match('=') ? TOKEN_PIPE_EQUALS : TOKEN_PIPE);
        case '!':
            return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '<':
            return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
            return makeToken(match('=') ?
                             TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"':
            return string('"');
        case '\'':
            return string('\'');
    }

    return errorToken("Unexpected character.");
}
