/* Dictu as a single file source library language.
 * Build against commit:
 * 1c6caafb6f08f8846a2d7fb0c8655a0af5c46f1f (HEAD -> develop, origin/develop, origin/HEAD)
 *
 * This tool works by parsing Dictu sources and generates the following files:
 *   dictu.c  
 * __dictu.h        # included by dictu.c
 *   opcodes.h      # included by dictu.c
 *   dictu.h        # included by the embedder
 *   main.c         # a source sample interpreter
 *   linenoise.[ch] # readline for the interpreter 
 *   Makefile       # a Makefile with the following targets:
 *     make library # builds Dictu as a shared library 
 *     make interpr # builds an interpreter    
 *
 * Usage:
 *  make clone-upstream   # clones the dictu sources (requires git)
 *  make update-upstream  # updates the cloned sources
 *  make                  # creates an executable named lmake, that generates the
 *                        # above files from the cloned dictu sources
 *                        # Note that the first two steps are optional if you already
 *                        # have Dictu sources in your filesystem
 *
 *  ./lmake
 *    options:
 *      --enable-http       # enable the http module (requires libcurl)
 *      --enable-sqlite     # enable sqlite functionality (requires libsqlite3)
 *      --build-library     # invokes make to build the library
 *      --build-interp      # invokes make to build the sample interpreter
 *      --clean-installed   # invokes make clean to clean installed generated objects
 *      --clean-build       # removes generated files from the build directory
 *      --enable-lai        # enable one language dialect, that it's syntax is in a more
 *                          # humanized form and mimics a bit of Lua
 *                          # Note: this is Dictu compatible, so Dictu scripts should
 *                          # run without any modification, otherwise is a bug
 *      --disable-exit      # disables exit() method from System Class
 *                          # off by default, enabled with lai implicitly
 *      --parse-lai         # parse lai script and output a Dictu script with a .du extension
 *                          # Note: When this option is encountered, parsing argv stops
 *                          # and any subsequent argunent is treated as argument to this
 *                          # function, which after its execution the program exits
 *                          # Also note that this is a crude algorithm
 *      --sysdir=`dir'      # system directory with write access, default [../sys]
 *      --builddir=`dir'    # build directory, default [build/dictu] or [build/lai]
 *      --langcdir=`dir'    # Dictu c sources directory, default [src/Dictu]
 *      --srcdir=`dir'      # source directory for this program, default [src]
 *      --donot-generate    # do not generate any files
 *      --donot-make-sysdir # do not make sys directory
 *      --help, -h          # show this message
 */

#define _POSIX_C_SOURCE 200809L

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#define ifnot(__expr__) if (0 == (__expr__))
#define bytelen strlen

#define uchar unsigned char

char *c_files[] = {
  "vm/common",
  "vm/value",
  "vm/chunk",
  "vm/table",
  "vm/object",
  "vm/scanner",
  "vm/compiler",
  "vm/vm",
  "vm/natives",
  "vm/memory",
  "vm/util"
};

char *dtype_files[] = {
  "bool",
  "class",
  "copy",
  "dicts",
  "files",
  "instance",
  "result",
  "lists/list-source",
  "lists/lists",
  "nil",
  "number",
  "sets",
  "strings",
};

char *opt_files[] = {
  "optionals",
  "base64/base64Lib",
  "base64",
  "c",
  "datetime",
  "env",
  "hashlib/constants",
  "hashlib/utils",
  "hashlib/sha256",
  "hashlib/hmac",
  "hashlib/bcrypt/bcrypt",
  "hashlib/bcrypt/blf",
  "hashlib/bcrypt/portable_endian",
  "hashlib/bcrypt/timingsafe_bcmp",
  "hashlib",
  "http",
  "json/jsonParseLib",
  "json/jsonBuilderLib",
  "json",
  "math",
  "path",
  "process",
  "random",
  "socket",
  "sqlite",
//  "sqlite/sqlite3",
  "system"
};

char *feature_macros = "#define _XOPEN_SOURCE 700\n\n";

char *std_headers[] = {
  "stdint",
  "stddef",
  "stdbool",
  "stdio",
  "stdlib",
  "string",
  "stdarg",
  "unistd",
  "ctype",
  "time",
  "math",
  "sys/utsname",
  "sys/stat",
  "sys/types",
  "sys/wait",
  "curl/curl",
  "sqlite3",
  "dirent",
  "arpa/inet",
  "errno",
  "assert",
};

char main_headers[] =
  "#include <stdint.h>\n"
  "#include <stddef.h>\n"
  "#include <stdbool.h>\n"
  "#include <stdio.h>\n"
  "#include <stdlib.h>\n"
  "#include <string.h>\n"
  "#include <math.h>\n";

#define ARRLEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define DICTU_NAME "dictu"
#define LAI_NAME "lai"
#define VERSION "0.3"

#define H_TYPE 0
#define C_TYPE 1

#define NO_APPEND 0
#define APPEND    1

#define MAKEFILE   "Makefile"
#define MAIN       "main.c"
#define DICTU_EXT  ".du"
#define DICTU_API  "dictu.h"
#define LAI_API    "lai.h"
#define LAI_EXTRA  "lai_identifierType.c"

#define MAKE_LIBRARY "make library"
#define MAKE_INTERP "make interp"
#define MAKE_CLEAN "make clean"

#define DIR_SEP           '/'
#define DIR_SEP_STR       "/"
#define IS_DIR_SEP(c)     (c == DIR_SEP)
#define ISNOT_DIR_SEP(c)  (c != DIR_SEP)

#define PARSELINE_OK         2
#define PARSELINE_NEXT_LINE  1
#define PARSELINE_NEXT_FILE  0
#define PARSELINE_BREAK     -1

#define PARSEFILE_OK         1
#define PARSEFILE_NEXT       0
#define PARSEFILE_BREAK     -1

typedef struct lang_t lang_t;

typedef int(*File_cb) (lang_t *, char *);
typedef int(*Line_cb) (lang_t *, char *, char *, size_t);

typedef struct lang_t {
  char
    *base_dir,
    *src_dir,
    *build_dir,
    *sys_dir,
    *lang_c_dir,
    *datatype_dir,
    *optional_dir,
    *lang_name,
     ext[4];

  int
    file_idx,
    arg_idx,
    exttype,
    enable_lai,
    enable_http,
    enable_sqlite,
    enable_repl,
    disable_exit,
    build_library,
    build_interp,
    donot_generate,
    clean_installed,
    clean_build,
    help,
    skip_function,
    lai_to_dictu,
    make_sys_dir;

  FILE *fp_out;

  size_t
    datatype_dir_len,
    optional_dir_len,
    lang_c_dir_len,
    src_dir_len,
    build_dir_len,
    lang_name_len,
    makefile_len,
    main_len,
    api_len,
    dictu_api_len,
    lai_api_len,
    lai_ext_len,
    base_dir_len,
    sys_dir_len,
    line_len;

  Line_cb line_cb;
  File_cb file_cb;
  File_cb on_close_cb;
} lang_t;


#define Alloc(size) ({                              \
  void *ptr__ = NULL;                               \
  ptr__ = calloc (1, (size));                       \
  if (NULL == ptr__) {                              \
    fprintf (stderr, "couldn't allocate memory\n"); \
    exit (1);                                       \
  }                                                 \
  ptr__;                                            \
  })

#define Realloc(ptr, size) ({                       \
  void *ptr__ = NULL;                               \
  ptr__ = realloc ((ptr), (size));                  \
  if (NULL == ptr__) {                              \
    fprintf (stderr, "couldn't allocate memory\n"); \
    exit (1);                                       \
  }                                                 \
  ptr__;                                            \
  })

int str_eq (const char *sa, const char *sb) {
  const uchar *spa = (const uchar *) sa;
  const uchar *spb = (const uchar *) sb;
  for (; *spa == *spb; spa++, spb++)
    if (*spa == 0) return 1;

  return 0;
}

int str_cmp_n (const char *sa, const char *sb, size_t n) {
  const uchar *spa = (const uchar *) sa;
  const uchar *spb = (const uchar *) sb;
  for (;n--; spa++, spb++) {
    if (*spa != *spb)
      return (*(uchar *) spa - *(uchar *) spb);

    if (*spa == 0) return 0;
  }

  return 0;
}

int str_eq_n  (const char *sa, const char *sb, size_t n) {
  return (0 == str_cmp_n (sa, sb, n));
}

size_t byte_cp (char *dest, const char *src, size_t nelem) {
  const char *sp = src;
  size_t len = 0;

  while (len < nelem && *sp) {
    dest[len] = *sp++;
    len++;
  }

  return len;
}

size_t str_cp (char *dest, size_t dest_len, const char *src, size_t nelem) {
  size_t num = (nelem > (dest_len - 1) ? dest_len - 1 : nelem);
  size_t len = (NULL == src ? 0 : byte_cp (dest, src, num));
  dest[len] = '\0';
  return len;
}

int is_directory (char *dname) {
  struct stat st;
  if (-1 == stat (dname, &st)) return 0;
  return S_ISDIR (st.st_mode);
}

int file_is_reg (const char *fname) {
  struct stat st;
  if (-1 == stat (fname, &st))
    return 0;
  return S_ISREG (st.st_mode);
}

char **dirlist (char *dir) {
  if (NULL == dir)
    return NULL;

  if (0 == is_directory (dir))
    return NULL;

  DIR *dh = NULL;
  if (NULL == (dh = opendir (dir)))
    return NULL;

  struct dirent *dp;

  size_t dirlen = bytelen (dir);
  size_t len;

  int num_files = 2;
  char **files = Alloc (num_files * sizeof (char *));

  int idx = 0;

  while (1) {
    errno = 0;

    if (NULL == (dp = readdir (dh)))
      break;

    len = bytelen (dp->d_name);

    if (len < 3 && dp->d_name[0] == '.')
      if (len == 1 || dp->d_name[1] == '.')
        continue;

    size_t buflen = dirlen +  len + 1;
    char buf[buflen + 1];
    snprintf (buf, buflen + 1, "%s/%s", dir, dp->d_name);

    if (0 == file_is_reg (buf))
      continue;

    if (idx == num_files) {
      num_files += num_files / 2;

      files = Realloc (files, num_files * sizeof (char *));
    }

    files[idx] = Alloc (buflen + 1);
    str_cp (files[idx++], buflen + 1, buf, buflen);
  }

  if (idx == num_files) {
    num_files++;

    files = Realloc (files, num_files * sizeof (char *));
  }

  files[num_files - 1] = NULL;

  closedir (dh);
  return files;
}

char *dir_current (void) {
  size_t size = 64;
  char *buf = Alloc (size);

  char *dir = NULL;

  while ((dir = getcwd (buf, size)) == NULL) {
    if (errno != ERANGE) break;
    size += (size / 2);
    buf = Realloc (buf, size);
  }

  return dir;
}

char *path_dirname (char *name) {
  size_t len = bytelen (name);
  char *dname = NULL;
  if (name == NULL || 0 == len) {
    dname = Alloc (2); dname[0] = '.'; dname[1] = '\0';
    return dname;
  }

  char *sep = name + len;

  while (sep != name) {
    if (0 == IS_DIR_SEP (*sep)) break;
    sep--;
  }

  while (sep != name) {
    if (IS_DIR_SEP (*sep)) break;
    sep--;
  }

  while (sep != name) {
    if (0 == IS_DIR_SEP (*sep)) break;
    sep--;
  }

  if (sep == name) {
    dname = Alloc (2);
    dname[0] = (IS_DIR_SEP (*name)) ? DIR_SEP : '.'; dname[1] = '\0';
    return dname;
  }

  len = sep - name + 1;
  dname = Alloc (len + 1);
  str_cp (dname, len + 1, name, len);
  return dname;
}

char *byte_in_str (const char *s, int c) {
  const char *sp = s;
  while (*sp != c) {
    if (*sp == 0) return NULL;
    sp++;
  }
  return (char *)sp;
}

char *nullbyte_in_str (const char *s) {
  return byte_in_str (s, 0);
}

char *path_basename (char *name) {
  if (NULL == name)
    return name;

  char *p = nullbyte_in_str (name);
  if (p == NULL) p = name + bytelen (name) + 1;
  if (p - 1 == name && IS_DIR_SEP (*(p - 1)))
    return p - 1;

  while (p > name && IS_DIR_SEP (*(p - 1))) p--;
  while (p > name && ISNOT_DIR_SEP (*(p - 1))) --p;
  if (p == name && IS_DIR_SEP (*p))
    return DIR_SEP_STR;
  return p;
}

char *path_extname (char *name) {
  if (name == NULL)
    return name;

  char *p = nullbyte_in_str (name);
  if (p == NULL) p = name + bytelen (name) + 1;
  while (p > name && (*(p - 1) != '.')) --p;
  if (p == name)
    return "";
  p--;
  return p;
}

int make_dir (char *);

int make_dir (char *path) {
  if (str_eq (path, "."))
    return 0;

  char *dname = path_dirname (path);

  int retval = 0;

  if (str_eq (dname, "/"))
    goto theend;

  if ((retval = make_dir (dname)) != 0)
    goto theend;

  if (mkdir (path, 0755) != 0 && errno != EEXIST) {
    fprintf (stderr, "mkdir(): %s\n%s\n", path, strerror (errno));
    retval = -1;
  }

theend:
  free (dname);
  return retval;
}

int copy_file (char *src, char *dest, int append) {
  FILE *src_fp = fopen (src, "r");
  if (NULL == src_fp) {
    fprintf (stderr, "fopen(): %s\n%s\n", src, strerror (errno));
    return -1;
  }

  FILE *dest_fp = fopen (dest, append == APPEND ? "a+" : "w");
  if (NULL == dest_fp) {
    fprintf (stderr, "fopen(): %s\n%s\n", dest, strerror (errno));
    return -1;
  }

  int retval = -1;

  size_t num_bytes = 0;
  size_t written = 0;
  size_t chunk = 4096;
  int atend = 0;

  for (;;) {
    char bytes[chunk];
    bytes[0] = '\0';
    num_bytes = fread (bytes, 1, chunk, src_fp);
    atend = chunk != num_bytes;
    if (atend)
      if (0 == feof (src_fp)) {
        fprintf (stderr, "feof(): %s\n", strerror (errno));
        goto theend;
      }

    if (0 == (num_bytes = fwrite (bytes, 1, num_bytes, dest_fp))) {
      fprintf (stderr, "fwrite(): %s\n", strerror (errno));
      goto theend;
    }

    written += num_bytes;

    if (atend) break;
  }

  retval = 0;

theend:
  fclose (src_fp);
  fclose (dest_fp);
  return retval;
}

char *insert_str (lang_t *this, char *line, size_t len, char *ptr, char *buf, size_t buflen) {
  if (len + buflen + 1 > this->line_len) {
    this->line_len += buflen + 1;
    line = Realloc (line, this->line_len);
  }

  ptrdiff_t diff = ptr - line;
  memmove (line + diff + buflen, ptr, len - diff);
  memcpy (line + diff, buf, buflen);
  line[len + buflen] = '\0';
  return line;
}

int file_cb (lang_t *this, char * file) {
  if (NULL != strstr (file, "hashlib/constants.c"))
    return PARSEFILE_NEXT;

  if (NULL != strstr (file, "hashlib/bcrypt/portable_endian.c"))
    return PARSEFILE_NEXT;

  if (NULL != strstr (file, "hashlib/bcrypt/timingsafe_bcmp.h"))
    return PARSEFILE_NEXT;

  if (NULL != strstr (file, "common.c"))
    return PARSEFILE_NEXT;

  if (NULL != strstr (file, "http")) {
    fprintf (this->fp_out, "\n\n#ifndef DISABLE_HTTP\n");
    return PARSEFILE_OK;
  }

  if (NULL != strstr (file, "list-source.c"))
    return PARSEFILE_NEXT;

  if (NULL != strstr (file, "sqlite3.h")) {
    char sqlite_dest[this->build_dir_len + 9 + 2];
    snprintf (sqlite_dest, this->build_dir_len + 9 + 2, "%s/%s",
        this->build_dir, "sqlite3.h");
    copy_file (file, sqlite_dest, NO_APPEND);
    return PARSEFILE_NEXT;
  }

  if (NULL != strstr (file, "sqlite"))
    fprintf (this->fp_out, "\n#ifndef DISABLE_SQLITE\n");

  return PARSEFILE_OK;
}

int parse_compiler (lang_t *this, char *line, size_t len) {
  (void) this;
  if (len < 5) return PARSELINE_OK;

  char prefix[] = "comp_";
  size_t pr_len = 5;

  char *tmp = strstr (line, "number(");
  if (tmp) {
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
  }

  tmp = strstr (line, "{number,");
  if (tmp) {
    tmp++;
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
  }

  tmp = strstr (line, "string(");
  if (tmp) {
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
  }

  tmp = strstr (line, "{string,");
  if (tmp) {
    tmp++;
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
  }

  tmp = strstr (line, "function(");
  if (tmp) {
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
  }

  tmp = strstr (line, "call(");
  if (tmp) {
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
  }

  tmp = strstr (line, ", call,");
  if (tmp) {
    tmp += 2;
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
   }

  return PARSELINE_OK;
}

int parse_scanner (lang_t *this, char *line, size_t len) {
  char prefix[] = "scan_";
  size_t pr_len = 5;

  char *tmp = strstr (line, "peek");
  while (tmp) {
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    len += pr_len;
    tmp = strstr(tmp + 8, "peek");
  }

  tmp = strstr (line, "advance");
  if (tmp)
    line = insert_str (this, line, len, tmp, prefix, pr_len);

  tmp = strstr (line, "match");
  if (tmp)
    line = insert_str (this, line, len, tmp, prefix, pr_len);

  if (this->enable_lai) {
    if (this->skip_function) {
      if (str_eq (line, "}\n"))
        this->skip_function = 0;
      return PARSELINE_NEXT_LINE;
    }

    if (str_eq_n (line, "static TokenType identifierType", 31)) {
      char ext[this->src_dir_len + this->lai_ext_len + 2];
      snprintf (ext, this->src_dir_len + this->lai_ext_len + 2, "%s/%s",
          this->src_dir, LAI_EXTRA);

      FILE *fp = fopen (ext, "r");
      if (fp == NULL) {
        fprintf (stderr, "fopen(): %s\n%s\n", ext, strerror (errno));
        return PARSELINE_BREAK;
      }

      if (-1 == fseek (fp, 0, SEEK_END)) {
        fprintf (stderr, "fseek(): %s\n", strerror (errno));
        fclose (fp);
        return PARSELINE_BREAK;
      }

      long bytes = ftell (fp);
      if (-1 == bytes) {
        fprintf (stderr, "ftell(): %s\n", strerror (errno));
        fclose (fp);
        return PARSELINE_BREAK;
      }

      if (-1 == fseek (fp, 0, SEEK_SET)) {
        fprintf (stderr, "fseek(): %s\n", strerror (errno));
        fclose (fp);
        return PARSELINE_BREAK;
      }

      char buf[bytes + 1];
      if ((size_t) bytes != fread (buf, 1, bytes, fp)) {
        fprintf (stderr, "fread(): couldn't read the requester bytes\n");
        fclose (fp);
        return PARSELINE_BREAK;
      }

      buf[bytes] = '\0';
      fprintf (this->fp_out, "%s", buf);
      this->skip_function = 1;
      fclose (fp);
      return PARSELINE_NEXT_LINE;
    }
  }

  return PARSELINE_OK;
}

int parse_class (lang_t *this, char *line, size_t len) {
  char prefix[] = "clas_";
  size_t pr_len = 5;

  char *tmp = strstr (line, "toString(");
  if (tmp) {
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
  }

  tmp = strstr (line, "toString)");
  if (tmp)
    line = insert_str (this, line, len, tmp, prefix, pr_len);

  return PARSELINE_OK;
}

int parse_env (lang_t *this, char *line, size_t len) {
  char prefix[] = "env_";
  size_t pr_len = 4;
  char *tmp = strstr (line, "get(");
  if (tmp) {
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    return PARSELINE_OK;
  }

  tmp = strstr (line, "get)");
  if (tmp)
    line = insert_str (this, line, len, tmp, prefix, pr_len);

  return PARSELINE_OK;
}

int parse_system (lang_t *this, char *line, size_t len) {
  char pat[] = "defineNative(vm, &klass->methods, \"exit\", exitNative);\n";
  size_t plen = bytelen (pat);

  if (len < plen)
    return PARSELINE_OK;

  ifnot (this->disable_exit)
    return PARSELINE_OK;


  char dis_txt[] = "/*** DISABLED ***/\n    (void) exitNative;\n    // ";
  size_t dis_len = bytelen (dis_txt);

  char *tmp = strstr (line, pat);
  if (tmp)
    line = insert_str (this, line, len, tmp, dis_txt, dis_len);

  return PARSELINE_OK;
}

int parse_jsonBuilderLib (lang_t *this, char *line, size_t len) {
  (void) this; (void) len;
  char *tmp = strstr (line, "default_opts");
  if (tmp) {
    tmp[0] = 'D';
    tmp[8] = 'O';
  }

  return PARSELINE_OK;
}

int parse_jsonParseLib (lang_t *this, char *line, size_t len) {
  char *tmp = strstr (line, "uchar");
  while (tmp) {
    if (*(tmp-1) != '_' || *(tmp+5) != '2') {
      line = insert_str (this, line, len, tmp, "l_", 2);
      len += 2;
    }

    tmp = strstr (tmp + 5, "uchar");
  }

  return PARSELINE_OK;
}

int parse_sqlite (lang_t *this, char *line, size_t len) {
  char prefix[] = "sqlite_";
  size_t pr_len = 7;

  char *tmp = strstr (line, "execute");
  while (tmp) {
    line = insert_str (this, line, len, tmp, prefix, pr_len);
    len += pr_len;
    tmp = strstr(tmp + 14, "execute");
  }

  return PARSELINE_OK;
}

int parse_optionals (lang_t *this, char *line, size_t len) {
  if (strstr (line, "Sqlite")) {
    line = insert_str (this, line, len, line,
        "#ifndef DISABLE_SQLITE\n", 23);

    char *ptr = line + len + 23;
    line = insert_str (this, line, len + 23, ptr,
     "#endif /* DISABLE_SQLITE */\n", 28);
  }

  return PARSELINE_OK;
}

int parse_datetime (lang_t *this, char *line, size_t len) {
  (void) this; (void) len; (void) len;
  return PARSELINE_OK;

  char *tmp = strstr (line, "XOPEN");
  if (tmp)
    return PARSELINE_NEXT_LINE;

  return PARSELINE_OK;
}

int line_cb (lang_t *this, char *file, char *line, size_t len) {
  if (0 == strncmp ("#include", line, 8))
    return PARSELINE_NEXT_LINE;

  if (strstr (file, "sqlite"))
    return parse_sqlite (this, line, len);

  if (strstr (file, "compiler.c"))
    return parse_compiler (this, line, len);

  if (strstr (file, "scanner.c"))
    return parse_scanner (this, line, len);

  if (strstr (file, "class.c"))
    return parse_class (this, line, len);

  if (strstr (file, "env.c"))
    return parse_env (this, line, len);

  if (strstr (file, "system.c"))
    return parse_system (this, line, len);

  if (strstr (file, "jsonBuilderLib.c"))
    return parse_jsonBuilderLib (this, line, len);

  if (strstr (file, "jsonParseLib.c"))
    return parse_jsonParseLib (this, line, len);

  if (strstr (file, "datetime.h"))
    return parse_datetime (this, line, len);

  if (strstr (file, "optionals.c"))
    return parse_optionals (this, line, len);

  return PARSELINE_OK;
}

int file_on_close_cb (lang_t *this, char *file) {
  if (strstr (file, "sqlite")) {
    fprintf (this->fp_out, "\n#endif /* DISABLE_SQLITE */\n");
    return PARSEFILE_OK;
  }

  if (strstr (file, "http")) {
    fprintf (this->fp_out, "\n#endif /* DISABLE_HTTP */\n");
    return PARSEFILE_OK;
  }

  if (strstr (file, "vm.c")) {
    fprintf (this->fp_out,
        "\n/*** EXTENSIONS ***/\n\n"
        "Table *vm_get_globals(DictuVM *vm) {\n"
        "    return &vm->globals;\n}\n\n"
        "size_t vm_sizeof(void) {\n"
        "    return sizeof(DictuVM);\n}\n\n"
        "ObjModule *vm_module_get(DictuVM *vm, char *name, int len) {\n"
        "    Value moduleVal;\n"
        "    ObjString *string = copyString (vm, name, len);\n"
        "    if (!tableGet(&vm->modules, string, &moduleVal))\n"
        "        return NULL;\n"
        "    return AS_MODULE(moduleVal);\n}\n\n"
        "Table *vm_get_module_table(DictuVM *vm, char *name, int len) {\n"
        "    ObjModule *module = vm_module_get(vm, name, len);\n"
        "    if (NULL == module)\n"
        "        return NULL;\n"
        "    return &module->values;\n}\n\n"
        "Value *vm_table_get_value(DictuVM *vm, Table *table, ObjString *obj, Value *value){\n"
        "    UNUSED(vm);\n"
        "    if (false == tableGet(table, obj, value))\n"
        "        return NULL;\n"
        "    return value;\n}\n\n"
        "/*** EXTENSIONS END ***/\n");
    return PARSEFILE_OK;
  }

  return PARSEFILE_OK;
}

int write_files (lang_t *this, char **files, size_t arrlen) {
  size_t files_len[arrlen];
  size_t max_size = 0;
  for (size_t i = 0; i < arrlen; i++) {
    files_len[i] = bytelen (files[i]);
    if (max_size < files_len[i])
      max_size = files_len[i];
  }

  size_t file_len = max_size + this->base_dir_len + 3;
  char file[file_len + 1];

  char *line = Alloc (4096);
  this->line_len = 4096;

  FILE *fp = NULL;

  for (size_t i = 0; i < arrlen; i++) {
    snprintf (file, file_len + 1, "%s/%s.%c",
        this->base_dir, files[i], this->ext[this->exttype]);

    int cb_retval = this->file_cb (this, file);
    if (PARSEFILE_BREAK == cb_retval) {
      return 0;
    } else if (PARSEFILE_NEXT == cb_retval) {
      continue;
    }

    if ((fp = fopen (file, "r")) == NULL) {
      fprintf (stderr, "fopen(): %s\n%s\n", file, strerror (errno));
      return -1;
    }

    fprintf (this->fp_out, "\n    /* %d: %s.%c */\n", ++this->file_idx, files[i], this->ext[this->exttype]);

    line[0] = '\0';

    ssize_t nread;
    while (-1 != (nread = getline (&line, &this->line_len, fp))) {
      if (nread) {
        line[nread] = '\0';
        cb_retval = this->line_cb (this, file, line, nread);
        if (PARSELINE_BREAK == cb_retval) {
          fclose (fp);
          free (line);
          return 0;
        }

        if (PARSELINE_NEXT_FILE == cb_retval)
          goto next_file;

        if (PARSELINE_NEXT_LINE == cb_retval)
          continue;

        fprintf (this->fp_out, "%s", line);
        fflush (this->fp_out);
      }
    }

next_file:
    this->on_close_cb (this, file);

    fclose (fp);
  }

  free (line);
  return 0;
}

int write_include_std_headers (lang_t *this) {
  fprintf (this->fp_out, "%s", feature_macros);

  for (size_t i = 0; i < ARRLEN(std_headers); i++) {
    if (str_eq (std_headers[i], "curl/curl")) {
      fprintf (this->fp_out,
          "#ifndef DISABLE_HTTP\n"
          "#include <%s.h>\n"
          "#endif /* DISABLE_HTTP */\n",
          std_headers[i]);
        continue;
    }

    if (str_eq (std_headers[i], "sqlite3")) {
      fprintf (this->fp_out,
          "#ifndef DISABLE_SQLITE\n"
          "#include <%s.h>\n"
          "#endif /* DISABLE_SQLITE */\n",
          std_headers[i]);
        continue;
    }
    fprintf (this->fp_out, "#include <%s.h>\n", std_headers[i]);
  }

  return 0;
}

int create_hfile (lang_t *this) {
  if (this->donot_generate)
    return 0;

  this->base_dir = this->lang_c_dir;
  this->base_dir_len = this->lang_c_dir_len;

  size_t dest_file_len = this->build_dir_len + this->lang_name_len + 5;
  char dest_file[dest_file_len + 1];
  snprintf (dest_file, dest_file_len + 1, "%s/__%s.h", this->build_dir, this->lang_name);

  if ((this->fp_out = fopen (dest_file, "w")) == NULL) {
    fprintf (stderr, "fopen(): %s\n%s\n", dest_file, strerror (errno));
    return -1;
  }

  fprintf (this->fp_out, "typedef struct _vm DictuVM;\n");
  fprintf (this->fp_out,
      "typedef enum {\n"
        "INTERPRET_OK,\n"
        "INTERPRET_COMPILE_ERROR,\n"
        "INTERPRET_RUNTIME_ERROR\n"
      "} DictuInterpretResult;\n");

  this->exttype = H_TYPE;
  if (-1 == write_files (this, c_files, ARRLEN(c_files))) {
    fclose (this->fp_out);
    return -1;
  }

  this->base_dir = this->datatype_dir;
  this->base_dir_len = this->datatype_dir_len;

  if (-1 == write_files (this, dtype_files, ARRLEN(dtype_files))) {
    fclose (this->fp_out);
    return -1;
  }

  this->base_dir = this->optional_dir;
  this->base_dir_len = this->optional_dir_len;

  int retval = write_files (this, opt_files, ARRLEN(opt_files));
  fclose (this->fp_out);

  return retval;
}

int create_cfile (lang_t *this) {
  if (this->donot_generate)
    return 0;

  this->exttype = C_TYPE;
  this->base_dir = this->lang_c_dir;
  this->base_dir_len = this->lang_c_dir_len;

  size_t dest_file_len = this->build_dir_len + this->lang_name_len + 3;
  char dest_file[dest_file_len + 1];
  snprintf (dest_file, dest_file_len + 1, "%s/%s.c", this->build_dir, this->lang_name);

  if ((this->fp_out = fopen (dest_file, "w")) == NULL) {
    fprintf (stderr, "fopen(): %s\n%s\n", dest_file, strerror (errno));
    return -1;
  }

  write_include_std_headers (this);

  fprintf (this->fp_out, "\n#include \"__%s.h\"\n", this->lang_name);

/*
  if (this->enable_sqlite != 0)
    fprintf (this->fp_out,
        "\n#ifndef DISABLE_SQLITE\n"
        "#include \"sqlite3.h\"\n"
        "#endif\n");
*/

  if (-1 == write_files (this, c_files, ARRLEN(c_files))) {
    fclose (this->fp_out);
    return -1;
  }

  this->base_dir = this->datatype_dir;
  this->base_dir_len = this->datatype_dir_len;

  if (-1 == write_files (this, dtype_files, ARRLEN(dtype_files))) {
    fclose (this->fp_out);
    return -1;
  }

  this->base_dir = this->optional_dir;
  this->base_dir_len = this->optional_dir_len;

  int retval = write_files (this, opt_files, ARRLEN(opt_files));

  fclose (this->fp_out);
  return retval;
}

int copy_files (lang_t *this) {

  char makefile_file_src[this->src_dir_len + this->makefile_len + 2];
  snprintf (makefile_file_src, this->src_dir_len + this->makefile_len + 2, "%s/%s",
      this->src_dir, MAKEFILE);
  char makefile_file_dest[this->build_dir_len + this->makefile_len + 2];
  snprintf (makefile_file_dest, this->build_dir_len + this->makefile_len + 2, "%s/%s",
      this->build_dir, MAKEFILE);

  FILE *mfp = fopen (makefile_file_dest, "w");
  if (NULL == mfp) {
    fprintf (stderr, "fopen(): %s\n%s\n", makefile_file_dest, strerror (errno));
    return -1;
  }

  fprintf (mfp, "NAME    := %s\nVERSION := %s\n\n",
      this->lang_name, VERSION);

  fprintf (mfp, "ENABLE_REPL := %d\n", this->enable_repl);
  fprintf (mfp, "ENABLE_HTTP := %d\n", this->enable_http);
  if (this->enable_sqlite == 0)
    fprintf (mfp, "DISABLE_SQLITE := $(shell ldconfig -v 2>/dev/null | grep sqlite3 >/dev/null; echo $$?)\n");
  else
    fprintf (mfp, "DISABLE_SQLITE := 0\n");

  fprintf (mfp, "\nSYSDIR  := sys\n");

  fclose (mfp);

  if (this->donot_generate)
    return copy_file (makefile_file_src, makefile_file_dest, APPEND);

  if (-1 ==copy_file (makefile_file_src, makefile_file_dest, APPEND))
    return -1;

  size_t opcodes_len = bytelen ("vm/opcodes.h");
  char opc_file_src[this->lang_c_dir_len + opcodes_len + 2];
  snprintf (opc_file_src, this->lang_c_dir_len + opcodes_len + 2, "%s/vm/opcodes.h", this->lang_c_dir);
  char opc_file_dest[this->build_dir_len + opcodes_len + 2];
  snprintf (opc_file_dest, this->build_dir_len + opcodes_len + 2, "%s/opcodes.h", this->build_dir);

  if (-1 == copy_file (opc_file_src, opc_file_dest, NO_APPEND))
    return -1;

  size_t lineno_len = bytelen ("cli/linenoise.h");
  char lineno_file_src[this->lang_c_dir_len + lineno_len + 2];
  snprintf (lineno_file_src, this->lang_c_dir_len + lineno_len + 2, "%s/cli/linenoise.h", this->lang_c_dir);
  char lineno_file_dest[this->build_dir_len + lineno_len + 2];
  snprintf (lineno_file_dest, this->build_dir_len + lineno_len + 2, "%s/linenoise.h", this->build_dir);

  if (-1 == copy_file (lineno_file_src, lineno_file_dest, NO_APPEND))
    return -1;

  lineno_file_src[this->lang_c_dir_len + lineno_len] = 'c';
  lineno_file_dest[(this->build_dir_len + lineno_len) - 4] = 'c';

  if (-1 == copy_file (lineno_file_src, lineno_file_dest, NO_APPEND))
    return -1;

  size_t argparse_len = bytelen ("cli/argparse.h");
  char argparse_file_src[this->lang_c_dir_len + argparse_len + 2];
  snprintf (argparse_file_src, this->lang_c_dir_len + argparse_len + 2, "%s/cli/argparse.h", this->lang_c_dir);
  char argparse_file_dest[this->build_dir_len + argparse_len + 2];
  snprintf (argparse_file_dest, this->build_dir_len + argparse_len + 2, "%s/argparse.h", this->build_dir);

  if (-1 == copy_file (argparse_file_src, argparse_file_dest, NO_APPEND))
    return -1;

  argparse_file_src[this->lang_c_dir_len + argparse_len] = 'c';
  argparse_file_dest[(this->build_dir_len + argparse_len) - 4] = 'c';

  if (-1 == copy_file (argparse_file_src, argparse_file_dest, NO_APPEND))
    return -1;

  char encodings[this->build_dir_len + 9 + 1 + 1];
  snprintf (encodings, this->build_dir_len + 9 + 1 + 1, "%s/encodings", this->build_dir);

  if (-1 == access (encodings, F_OK)) {
    if (-1 == mkdir (encodings, 0755)) {
      fprintf (stderr, "mkdir(): %s\n%s\n", encodings, strerror (errno));
      return -1;
    }
  } else {
    if (0 == is_directory (encodings)) {
      fprintf (stderr, "%s: not a directory\n", encodings);
      return -1;
    }
  }

  size_t encodings_len = bytelen ("cli/encodings/utf8.h");
  char encodings_file_src[this->lang_c_dir_len + encodings_len + 3];
  snprintf (encodings_file_src, this->lang_c_dir_len + encodings_len + 3, "%s/cli/encodings/utf8.h", this->lang_c_dir);
  char encodings_file_dest[this->build_dir_len + encodings_len + 3];
  snprintf (encodings_file_dest, this->build_dir_len + encodings_len + 3, "%s/encodings/utf8.h", this->build_dir);

  if (-1 == copy_file (encodings_file_src, encodings_file_dest, NO_APPEND))
    return -1;

  encodings_file_src[this->lang_c_dir_len + encodings_len] = 'c';
  encodings_file_dest[(this->build_dir_len + encodings_len) - 4] = 'c';

  if (-1 == copy_file (encodings_file_src, encodings_file_dest, NO_APPEND))
    return -1;

  char main_file_src[this->src_dir_len + this->main_len + 2];
  snprintf (main_file_src, this->src_dir_len + this->main_len + 2, "%s/%s",
      this->src_dir, MAIN);
  char main_file_dest[this->build_dir_len + this->main_len + 2];
  snprintf (main_file_dest, this->build_dir_len + this->main_len + 2, "%s/%s",
      this->build_dir, MAIN);

  FILE *fp = fopen (main_file_dest, "w");

  if (NULL == fp) {
    fprintf (stderr, "fopen(): %s\n%s\n", main_file_dest, strerror (errno));
    return -1;
  }

  fprintf (fp, "%s\n", main_headers);
  fprintf (fp, "#include <%s.h>\n", this->lang_name);
  fclose (fp);
  if (-1 == copy_file (main_file_src, main_file_dest, APPEND))
    return -1;

  char api_file_src[this->src_dir_len + this->dictu_api_len + 2];
  snprintf (api_file_src, this->src_dir_len + this->dictu_api_len + 2, "%s/%s",
      this->src_dir, DICTU_API);
  char api_file_dest[this->build_dir_len + this->api_len + 2];
  snprintf (api_file_dest, this->build_dir_len + this->api_len + 2, "%s/%s",
      this->build_dir, this->enable_lai ? LAI_API : DICTU_API);

  if (-1 == copy_file (api_file_src, api_file_dest, NO_APPEND))
    return -1;

  size_t license_len = bytelen ("LICENSE");
  char license_file_src[this->lang_c_dir_len + license_len + 5];
  snprintf (license_file_src, this->lang_c_dir_len + license_len + 5, "%s/../LICENSE", this->lang_c_dir);
  char license_file_dest[this->build_dir_len + license_len + 2];
  snprintf (license_file_dest, this->build_dir_len + license_len + 2, "%s/LICENSE", this->build_dir);

  return copy_file (license_file_src, license_file_dest, NO_APPEND);
}

int enable_functionality (lang_t *this) {
  if (this->donot_generate)
    return 0;

  char makefile_file_dest[this->build_dir_len + this->makefile_len + 2];
  snprintf (makefile_file_dest, this->build_dir_len + this->makefile_len + 2, "%s/%s",
      this->build_dir, MAKEFILE);

  FILE *fp = fopen (makefile_file_dest, "r+");
  if (NULL == fp) {
    fprintf (stderr, "fopen(): %s\n%s\n", makefile_file_dest, strerror (errno));
    return -1;
  }

  char *line = NULL;
  int retval = -1;
  size_t len = 0;

  ssize_t nread;
  int checked = 0;
  while (-1 != (nread = getline (&line, &len, fp))) {
    if (nread) {
      if (this->enable_http && checked == 0) {
        if (str_eq (line, "ENABLE_HTTP := 0\n")) {
          if (-1 == fseek (fp, -2, SEEK_CUR)) {
            fprintf (stderr, "fseek(): %s\n", strerror (errno));
            goto theend;
          }

          fprintf (fp, "%d", 1);
          if (this->enable_repl == 0)
            break;

          if (-1 == fseek (fp, 2, SEEK_CUR)) {
            fprintf (stderr, "fseek(): %s\n", strerror (errno));
            goto theend;
          }

          checked++;
          continue;
        }
      }

      if (this->enable_repl) {
        if (str_eq (line, "ENABLE_REPL := 0\n")) {
          if (-1 == fseek (fp, -2, SEEK_CUR)) {
            fprintf (stderr, "fseek(): %s\n", strerror (errno));
            goto theend;
          }

          fprintf (fp, "%d", 1);
          break;
        }
      }

      if (this->enable_sqlite) {
        if (str_eq (line, "ENABLE_SQLITE := 0\n")) {
          if (-1 == fseek (fp, -2, SEEK_CUR)) {
            fprintf (stderr, "fseek(): %s\n", strerror (errno));
            goto theend;
          }

          fprintf (fp, "%d", 1);
          break;
        }
      }
    }
  }

  retval = 0;

theend:
  if (line != NULL)
    free (line);
  fclose (fp);
  return retval;
}

int make (lang_t *this) {
  if (0 == this->build_library &&
      0 == this->build_interp  &&
      0 == this->clean_installed) {
    return 0;
  }

  char *cwd = dir_current ();
  if (NULL == cwd) {
    fprintf (stderr, "can not determinate current directory\n");
    return -1;
  }

  int retval = 0;

  if (-1 == chdir (this->build_dir)) {
    fprintf (stderr, "chdir(): %s\n%s\n", this->build_dir, strerror (errno));
    retval = -1;
    goto theend;
  }

  int status = 0;

  if (this->clean_installed) {
    status = system (MAKE_CLEAN);

    if (WIFEXITED (status))
      status = WEXITSTATUS (status);

    if (0 != status) {
      retval = -1;
      goto theend;
    }
  }

  if (this->build_library) {
    status = system (MAKE_LIBRARY);

    if (WIFEXITED (status))
      status = WEXITSTATUS (status);

    if (0 != status) {
      retval = -1;
      goto theend;
    }
  }

  if (this->build_interp) {
    status = system (MAKE_INTERP);

    if (WIFEXITED (status))
      retval = WEXITSTATUS (status);
  }

theend:
  chdir (cwd);
  free (cwd);
  return retval;
}

int parse_lai (char *laiscript, size_t scr_len) {
  if (0 != access (laiscript, R_OK|F_OK)) {
    fprintf (stderr, "access(): %s\n%s\n", laiscript, strerror (errno));
    return -1;
  }

  char fname[scr_len + 1];
  snprintf (fname, scr_len + 1, "%s", laiscript);

  char *dname = path_dirname (fname);
  char *bname = path_basename (fname);
  char *extname = path_extname (fname);

  size_t dname_len = bytelen (dname);
  size_t bname_len = bytelen (bname);
  size_t extnm_len = bytelen (extname);

  bname[bname_len - extnm_len] = '\0';
  size_t len = ((dname_len + bname_len + bytelen (DICTU_EXT)) - extnm_len) + 1;
  char dict_file[len + 1];
  snprintf (dict_file, len + 1, "%s/%s%s", dname, bname, DICTU_EXT);
  free (dname);

  int retval = 0;
  FILE *src_fp = NULL;
  FILE *dest_fp = NULL;

  src_fp = fopen (laiscript, "r");
  if (NULL == src_fp) {
    fprintf (stderr, "fopen(): %s\n%s\n", laiscript, strerror (errno));
    retval = -1;
    goto theend;
  }

  dest_fp = fopen (dict_file, "w");
  if (NULL == dest_fp) {
    fprintf (stderr, "fopen(): %s\n%s\n", dict_file, strerror (errno));
    retval = -1;
    goto theend;
  }

  size_t lline_len = 256;
  char *lline = Alloc (lline_len);

  len = 0;
  char *line = NULL;

  ssize_t nread;
  while (-1 != (nread = getline (&line, &len, src_fp))) {
    if (nread) {
      if (len + 32 > lline_len) {
        lline_len = len + 32 + 1;
        lline = Realloc (lline, lline_len);
      }

      char *match;
      char tmp[lline_len];
      size_t tmp_len;

      ptrdiff_t diff = 0;
      ptrdiff_t ldiff = 0;

      tmp_len = str_cp (lline, lline_len, line, nread);
      tmp_len = str_cp (tmp, lline_len, lline, tmp_len);

      while ((match = strstr (tmp + diff, " is "))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 4;
        lline[ldiff] = ' ';
        lline[++ldiff] = '=';
        lline[++ldiff] = '=';
        lline[++ldiff] = ' ';
        lline[++ldiff] = '\0';
      }

      tmp_len = str_cp (lline + ldiff, lline_len, tmp + diff, tmp_len - diff);
      tmp_len = str_cp (tmp, lline_len, lline, tmp_len + ldiff);

      diff = ldiff = 0;
      while ((match = strstr (tmp + diff, " isnot "))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 7;
        lline[ldiff] = ' ';
        lline[++ldiff] = '!';
        lline[++ldiff] = '=';
        lline[++ldiff] = ' ';
        lline[++ldiff] = '\0';
      }

      tmp_len = str_cp (lline + ldiff, lline_len, tmp + diff, tmp_len - diff);
      tmp_len = str_cp (tmp, lline_len, lline, tmp_len + ldiff);

      diff = ldiff = 0;
      while ((match = strstr (tmp + diff, "orelse "))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 7;
        lline[ldiff]   = '}';
        lline[++ldiff] = ' ';
        lline[++ldiff] = 'e';
        lline[++ldiff] = 'l';
        lline[++ldiff] = 's';
        lline[++ldiff] = 'e';
        lline[++ldiff] = ' ';
        lline[++ldiff] = '\0';
      }

      tmp_len = str_cp (lline + ldiff, lline_len, tmp + diff, tmp_len - diff);
      tmp_len = str_cp (tmp, lline_len, lline, tmp_len + ldiff);

      diff = ldiff = 0;
      while ((match = strstr (tmp + diff, "forever do"))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 10;
        lline[ldiff]   = 'w';
        lline[++ldiff] = 'h';
        lline[++ldiff] = 'i';
        lline[++ldiff] = 'l';
        lline[++ldiff] = 'e';
        lline[++ldiff] = ' ';
        lline[++ldiff] = '(';
        lline[++ldiff] = 't';
        lline[++ldiff] = 'r';
        lline[++ldiff] = 'u';
        lline[++ldiff] = 'e';
        lline[++ldiff] = ')';
        lline[++ldiff] = ' ';
        lline[++ldiff] = '{';
        lline[++ldiff] = '\0';
      }

      tmp_len = str_cp (lline + ldiff, lline_len, tmp + diff, tmp_len - diff);
      tmp_len = str_cp (tmp, lline_len, lline, tmp_len + ldiff);

      diff = ldiff = 0;
      while ((match = strstr (tmp + diff, " beg "))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 5;
        lline[ldiff] = ' ';
        lline[++ldiff] = '{';
        lline[++ldiff] = ' ';
        lline[++ldiff] = '\0';
      }

      tmp_len = str_cp (lline + ldiff, lline_len, tmp + diff, tmp_len - diff);
      tmp_len = str_cp (tmp, lline_len, lline, tmp_len + ldiff);

      diff = ldiff = 0;
      while ((match = strstr (tmp + diff, " then "))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 6;
        lline[ldiff] = ' ';
        lline[++ldiff] = '{';
        lline[++ldiff] = ' ';
        lline[++ldiff] = '\0';
      }

      tmp_len = str_cp (lline + ldiff, lline_len, tmp + diff, tmp_len - diff);
      tmp_len = str_cp (tmp, lline_len, lline, tmp_len + ldiff);

      diff = ldiff = 0;
      while ((match = strstr (tmp + diff, " then\n"))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 6;
        lline[ldiff] = ' ';
        lline[++ldiff] = '{';
        lline[++ldiff] = '\n';
        lline[++ldiff] = '\0';
        goto print;
      }

      while ((match = strstr (tmp + diff, " do\n"))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 4;
        lline[ldiff] = ' ';
        lline[++ldiff] = '{';
        lline[++ldiff] = '\n';
        lline[++ldiff] = '\0';
        goto print;
      }

      while ((match = strstr (tmp + diff, "end\n"))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 4;
        lline[ldiff] = '}';
        lline[++ldiff] = '\n';
        lline[++ldiff] = '\0';
        goto print;
      }

      while ((match = strstr (tmp + diff, "beg\n"))) {
        ldiff = match - tmp;
        str_cp (lline + diff, lline_len, tmp, ldiff);
        diff = ldiff + 4;
        lline[ldiff] = '{';
        lline[++ldiff] = '\n';
        lline[++ldiff] = '\0';
        goto print;
      }

      str_cp (lline + ldiff, lline_len, tmp + diff, bytelen (tmp) - diff);
      str_cp (tmp, lline_len, lline, bytelen (lline));

print:
      fprintf (dest_fp, "%s", lline);

    }
  }


theend:
  if (line != NULL)
    free (line);

  if (NULL != src_fp)
    fclose (src_fp);

  if (NULL != dest_fp)
    fclose (dest_fp);

  return retval;
}

int parse_lai_to_dictu (lang_t *this, int argc, char **argv) {
  for (int i = this->arg_idx; i < argc; i++) {
    if (-1 == parse_lai (argv[i], bytelen (argv[i])))
      return -1;
  }

  return 0;
}

int show_help (char *prog) {
  fprintf (stdout,
     "Usage: %s [options]\n\n"
     "Options:\n"
     "  --enable-http       # enable the http module (requires libcurl)\n"
     "  --enable-sqlite     # enable sqlite3 module (requires libsqlite3)\n"
     "  --build-library     # invokes make to build the library\n"
     "  --build-interp      # invokes make to build the sample interpreter\n"
     "  --clean-installed   # invokes make clean to clean installed generated objects\n"
     "  --clean-build       # removes generated files from the build directory\n"
     "  --enable-lai        # enable lai dialect, that mimics Lua syntax."
     "                        Note that this is Dictu compatible, so Dictu scripts should"
     "                        run without any modification, otherwise is a bug\n"
     "  --disable-exit      # disables exit() method from System Class.\n"
     "                        This is off by default, enabled with --enable-lai implicitly\n"
     "  --parse-lai         # parse lai script and output a Dictu script with a .du extension.\n"
     "                        Note: When this option is encountered, it stops to parsing thargv list\n"
     "                        and any subsequent argunent is treated as argument to this\n"
     "                        function, which after its execution the program exits.\n"
     "                        This is a crude algorithm\n"
     "  --sysdir=`dir'      # system directory with write access, default [../sys]\n"
     "  --builddir=`dir'    # build directory, default [build/dictu] or [build/lai]\n"
     "  --langcdir=`dir'    # Dictu c sources directory, defualt [src/Dictu]\n"
     "  --srcdir=`dir'      # source directory for this program, default [src]\n"
     "  --donot-generate    # do not generate any files\n"
     "  --donot-make-sysdir # do not make sys directory\n"
     "  --help, -h          # show this message\n",
     prog);
  return 0;
}

int parse_args (lang_t *this, int argc, char **argv) {

  for (int i = 1; i < argc; i++) {
    if (str_eq (argv[i], "--enable-http")) {
      this->enable_http = 1;
      continue;
    }

    if (str_eq (argv[i], "--enable-sqlite")) {
      this->enable_sqlite = 1;
      continue;
    }

/*
    if (str_eq (argv[i], "--enable-repl")) {
      this->enable_repl = 1;
      continue;
    }
*/

    if (str_eq (argv[i], "--disable-exit")) {
      this->disable_exit = 1;
      continue;
    }

    if (str_eq (argv[i], "--enable-lai")) {
      this->api_len = this->lai_api_len;
      this->enable_lai = 1;

      /* disable exit by default */
      this->disable_exit = 1;

      this->lang_name_len = bytelen (LAI_NAME);
      this->lang_name = Alloc (this->lang_name_len + 1);
      snprintf (this->lang_name, this->lang_name_len + 1, "%s", LAI_NAME);
      continue;
    }

    if (str_eq (argv[i], "--help") || str_eq (argv[i], "-h")) {
      this->help = 1;
      continue;
    }

    if (str_eq (argv[i], "--build-library")) {
      this->build_library = 1;
      continue;
    }

    if (str_eq (argv[i], "--build-interp")) {
      this->build_interp = 1;
      continue;
    }

    if (str_eq (argv[i], "--clean-installed")) {
      this->clean_installed = 1;
      continue;
    }

    if (str_eq (argv[i], "--clean-build")) {
      this->clean_build = 1;
      continue;
    }

    if (str_eq (argv[i], "--donot-generate")) {
      this->donot_generate = 1;
      continue;
    }

    if (str_eq (argv[i], "--donot-make-sysdir")) {
      this->make_sys_dir = 0;
      continue;
    }

    if (str_eq_n (argv[i], "--sysdir=", 9)) {
      size_t len = bytelen (argv[i]) - 9;
      if (0 == len) {
        fprintf (stderr, "--sysdir= is an empty string\n");
        return -1;
      }

      this->sys_dir = Alloc (len + 1);
      snprintf (this->sys_dir, len + 1, "%s", argv[i] + 9);
      this->sys_dir_len = len;
      continue;
    }

    if (str_eq_n (argv[i], "--langcdir=", 11)) {
      size_t len = bytelen (argv[i]) - 11;
      if (0 == len) {
        fprintf (stderr, "--langcdir= is an empty string\n");
        return -1;
      }

      this->lang_c_dir = Alloc (len + 1);
      snprintf (this->lang_c_dir, len + 1, "%s", argv[i] + 11);
      this->lang_c_dir_len = len;
      continue;
    }

    if (str_eq_n (argv[i], "--builddir=", 11)) {
      size_t len = bytelen (argv[i]) - 11;
      if (0 == len) {
        fprintf (stderr, "--builddir= is an empty string\n");
        return -1;
      }

      this->build_dir = Alloc (len + 1);
      snprintf (this->build_dir, len + 1, "%s", argv[i] + 11);
      this->build_dir_len = len;
      continue;
    }

    if (str_eq_n (argv[i], "--srcdir=", 9)) {
      size_t len = bytelen (argv[i]) - 9;
      if (0 == len) {
        fprintf (stderr, "--srcdir= is an empty string\n");
        return -1;
      }

      this->src_dir = Alloc (len + 1);
      snprintf (this->src_dir, len + 1, "%s", argv[i] + 11);
      this->src_dir_len = len;
      continue;
    }

    if (str_eq (argv[i], "--parse-lai")) {
      this->lai_to_dictu = 1;
      this->arg_idx = i + 1;
      return 0;
    }

    fprintf (stderr, "Unknown option: %s\n", argv[i]);
    return -1;
  }

  return 0;
}

int make_dirs (lang_t *this) {
  if (-1 == make_dir (this->build_dir))
    return -1;

  if (this->make_sys_dir)
    if (-1 == make_dir (this->sys_dir))
      return -1;

  return 0;
}

int check_dirs (lang_t *this) {
  if (NULL == this->sys_dir) {
#ifndef SYSDIR
    fprintf (stderr, "SYSDIR is not defined, nor --sysdir= option specified\n");
    return -1;
#else
    this->sys_dir_len = bytelen (SYSDIR);
    this->sys_dir = Alloc (this->sys_dir_len + 1);
    snprintf (this->sys_dir, this->sys_dir_len + 1, "%s", SYSDIR);
#endif
  }

  if (NULL == this->lang_c_dir) {
#ifndef LANGCDIR
    fprintf (stderr, "LANGCDIR is not defined, nor --langcdir= option specified\n");
    return -1;
#else
    this->lang_c_dir_len = bytelen (LANGCDIR);
    this->lang_c_dir = Alloc (this->lang_c_dir_len + 1);
    snprintf (this->lang_c_dir, this->lang_c_dir_len + 1, "%s", LANGCDIR);
#endif
  }

  if (NULL == this->build_dir) {
#ifndef BUILDDIR
    fprintf (stderr, "BUILDDIR is not defined, nor --builddir= option specified\n");
    return -1;
#else
    this->build_dir_len = bytelen (BUILDDIR) + this->lang_name_len + 1;
    this->build_dir = Alloc (this->build_dir_len + 1);
    snprintf (this->build_dir, this->build_dir_len + 1, "%s/%s", BUILDDIR, this->lang_name);
#endif
  }

  if (NULL == this->src_dir) {
#ifndef SRCDIR
    fprintf (stderr, "SRCDIR is not defined, nor --srcdir= option specified\n");
    return -1;
#else
    this->src_dir_len = bytelen (SRCDIR);
    this->src_dir = Alloc (this->src_dir_len + 1);
    snprintf (this->src_dir, this->src_dir_len + 1, "%s", SRCDIR);
#endif
  }

  return make_dirs (this);
}

void deinit_this (lang_t *this) {
  if (this->sys_dir)
    free (this->sys_dir);

  if (this->lang_c_dir)
    free (this->lang_c_dir);

  if (this->datatype_dir)
    free (this->datatype_dir);

  if (this->optional_dir)
    free (this->optional_dir);

  if (this->src_dir)
    free (this->src_dir);

  if (this->build_dir)
    free (this->build_dir);

  if (this->lang_name)
    free (this->lang_name);
}

lang_t init_this (int argc, char **argv) {
  lang_t this;
  this.enable_http = 0;
  this.enable_repl = 1;
  this.enable_sqlite = 0;
  this.enable_lai  = 0;
  this.disable_exit = 0;
  this.help = 0;
  this.skip_function = 0;
  this.build_library = 0;
  this.build_interp  = 0;
  this.clean_build   = 0;
  this.clean_installed = 0;
  this.lai_to_dictu = 0;

  this.donot_generate = 0;
  this.sys_dir = NULL;
  this.lang_c_dir = NULL;
  this.build_dir = NULL;
  this.datatype_dir = NULL;
  this.optional_dir = NULL;
  this.src_dir = NULL;
  this.lang_name = NULL;

  if (-1 == parse_args (&this, argc, argv)) {
    deinit_this (&this);
    exit (1);
  }

  if (this.lai_to_dictu) {
    int retval = parse_lai_to_dictu (&this, argc, argv);
    deinit_this (&this);
    exit (retval);
  }

  if (NULL == this.lang_name) {
    this.lang_name_len = bytelen (DICTU_NAME);
    this.lang_name = Alloc (this.lang_name_len + 1);
    snprintf (this.lang_name, this.lang_name_len + 1, "%s", DICTU_NAME);
  }

  if (-1 == check_dirs (&this)) {
    deinit_this (&this);
    exit (1);
  }

  if (this.clean_build) {
    char **files = dirlist (this.build_dir);
    int i = 0;
    while (files[i]) {
      if (-1 == unlink (files[i])) {
        fprintf (stderr, "unlink(): %s\n%s\n", files[i-1], strerror (errno));
        deinit_this (&this);
        exit (1);
      }

      free (files[i++]);
    }

    free (files);
  }

  this.datatype_dir_len = this.lang_c_dir_len + 13;
  this.datatype_dir = Alloc (this.datatype_dir_len + 1);
  snprintf (this.datatype_dir, this.datatype_dir_len + 1, "%s/vm/datatypes",
     this.lang_c_dir);

  this.optional_dir_len = this.lang_c_dir_len + 10;
  this.optional_dir = Alloc (this.optional_dir_len + 1);
  snprintf (this.optional_dir, this.optional_dir_len + 1, "%s/optionals",
     this.lang_c_dir);

  this.makefile_len = bytelen (MAKEFILE);
  this.main_len = bytelen (MAIN);
  this.dictu_api_len = this.api_len = bytelen (DICTU_API);
  this.lai_api_len = bytelen (LAI_API);
  this.lai_ext_len = bytelen (LAI_EXTRA);
  this.ext[H_TYPE] = 'h'; this.ext[C_TYPE] = 'c'; this.ext[2] = '\0';
  this.file_idx = 0;
  this.make_sys_dir = 1;
  this.line_cb = line_cb;
  this.file_cb = file_cb;
  this.on_close_cb = file_on_close_cb;
  return this;
}

int run (lang_t *this, char *prog) {
  if (this->help)
    return show_help (prog);

  if (-1 == create_cfile (this))
    return 1;

  if (-1 == create_hfile (this))
    return 1;

  if (-1 == copy_files (this))
    return 1;

  if (-1 == enable_functionality (this))
    return 1;

  if (0 != make (this))
    return 1;

  return 0;
}

int main (int argc, char **argv) {
  lang_t this = init_this (argc, argv);

  int retval = run (&this, argv[0]);

  deinit_this (&this);

  return retval;
}
