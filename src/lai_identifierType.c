static TokenType identifierType(Scanner *scanner) {
    switch (scanner->start[0]) {
        case 'a':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'b': {
                        return checkKeyword(scanner, 2, 6, "stract", TOKEN_ABSTRACT);
                    }

                    case 'n': {
                        return checkKeyword(scanner, 2, 1, "d", TOKEN_AND);
                    }

                    case 's': {
                        return checkKeyword(scanner, 2, 0, "", TOKEN_AS);
                    }
                }
            }
            break;
        case 'b':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'e':
                        return checkKeyword(scanner, 2, 1, "g", TOKEN_LEFT_BRACE);
                    case 'r':
                        return checkKeyword(scanner, 2, 3, "eak", TOKEN_BREAK);
                 }
             }
             break;
        case 'c':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'l':
                        return checkKeyword(scanner, 2, 3, "ass", TOKEN_CLASS);
                    case 'o':
                        // Skip second char
                        // Skip third char
                        if (scanner->current - scanner->start > 3) {
                            switch (scanner->start[3]) {
                                case 't':
                                    return checkKeyword(scanner, 4, 4, "inue", TOKEN_CONTINUE);
                                case 's':
                                    return checkKeyword(scanner, 4, 1, "t", TOKEN_CONST);
                            }
                        }

                }
            }
            break;
        case 'd':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'e':
                        return checkKeyword(scanner, 2, 1, "f", TOKEN_DEF);
                    case 'o':
                        return checkKeyword(scanner, 1, 1, "o", TOKEN_LEFT_BRACE);
                }
            }
            break;
        case 'e':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'n':
                        return checkKeyword(scanner, 2, 1, "d", TOKEN_RIGHT_BRACE);
                    case 'l':
                        return checkKeyword(scanner, 2, 2, "se", TOKEN_ELSE);
                }
            }
            break;
        case 'f':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'a':
                        return checkKeyword(scanner, 2, 3, "lse", TOKEN_FALSE);
                    case 'r':
                        return checkKeyword(scanner, 2, 2, "om", TOKEN_FROM);
                    case 'o':
                        if (scanner->current - scanner->start == 3)
                            return checkKeyword(scanner, 2, 1, "r", TOKEN_FOR);

                        if (TOKEN_IDENTIFIER == checkKeyword(scanner, 2, 5, "rever", 0))
                            return TOKEN_IDENTIFIER;

                        char *modified = (char *) scanner->start;
                        char replaced[] = "while (1) {";
                        for (int i = 0; i < 11; i++)
                            modified[i] = replaced[i];
                        scanner->start   = (const char *) modified;
                        scanner->current = scanner->start + 5;
                        return TOKEN_WHILE;
                }
            }
            break;
        case 'i':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'f':
                        return checkKeyword(scanner, 2, 0, "", TOKEN_IF);
                    case 'm':
                        return checkKeyword(scanner, 2, 4, "port", TOKEN_IMPORT);
                    case 's':
                      if (scanner->current - (scanner->start + 1) > 1)
                          return checkKeyword(scanner, 2, 3, "not", TOKEN_BANG_EQUAL);
                      return checkKeyword(scanner, 1, 1, "s", TOKEN_EQUAL_EQUAL);
                 }
            }
            break;
        case 'n':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'o':
                        return checkKeyword(scanner, 2, 1, "t", TOKEN_BANG);
                    case 'i':
                        return checkKeyword(scanner, 2, 1, "l", TOKEN_NIL);
                }
            }
            break;
        case 'o':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'r':
                        if (scanner->current - (scanner->start + 1) > 1) {
                            if (TOKEN_ELSE != checkKeyword(scanner, 2, 4, "else", TOKEN_ELSE))
                                return TOKEN_IDENTIFIER;

                            char *modified = (char *) scanner->start;
                            modified[0] = '}'; modified[1] = ' ';
                            scanner->start   = (const char *) modified;
                            scanner->current = scanner->start + 1;
                            return TOKEN_RIGHT_BRACE;
                        }
                    return checkKeyword(scanner, 1, 1, "r", TOKEN_OR);
                }
             }
             break;
        case 'r':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'e':
                        return checkKeyword(scanner, 2, 4, "turn", TOKEN_RETURN);
                }
            } else {
                if (scanner->start[1] == '"' || scanner->start[1] == '\'') {
                    scanner->rawString = true;
                    return TOKEN_R;
                }
            }
            break;
        case 's':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'u':
                        return checkKeyword(scanner, 2, 3, "per", TOKEN_SUPER);
                    case 't':
                        return checkKeyword(scanner, 2, 4, "atic", TOKEN_STATIC);
                }
            }
            break;
        case 't':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'h':
                  	  if (scanner->current - scanner->start > 1) {
                            switch (scanner->start[2]) {
                                case 'e':
                                    return checkKeyword(scanner, 3, 1, "n", TOKEN_LEFT_BRACE);
                                case 'i':
                                    return checkKeyword(scanner, 3, 1, "s", TOKEN_THIS);
                            }
                        }
                        break;
                    case 'r':
                        if (scanner->current - scanner->start > 1) {
                            switch (scanner->start[2]) {
                                case 'u':
                                    return checkKeyword(scanner, 3, 1, "e", TOKEN_TRUE);
                                case 'a':
                                    return checkKeyword(scanner, 3, 2, "it", TOKEN_TRAIT);
                            }
                        }
                    break;
                }
            }
            break;
        case 'u':
            return checkKeyword(scanner, 1, 2, "se", TOKEN_USE);
        case 'v':
            return checkKeyword(scanner, 1, 2, "ar", TOKEN_VAR);
        case 'w':
            if (scanner->current - scanner->start > 1) {
                switch (scanner->start[1]) {
                    case 'h':
                        return checkKeyword(scanner, 2, 3, "ile", TOKEN_WHILE);
                    case 'i':
                        return checkKeyword(scanner, 2, 2, "th", TOKEN_WITH);
                }
            }
            break;
    }

    return TOKEN_IDENTIFIER;
}
