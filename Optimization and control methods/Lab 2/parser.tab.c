/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

/* Исходные данные */
int n, m;
char direction[4];
double *c;
double **A;
double *b;
char **r;      /* массив строк длины m */
char **sigma;  /* массив строк длины n */

/* Флаги парсинга */
int parsing_matrix_A = 0;
int parsing_vector_c = 0;
int parsing_vector_b = 0;
int parsing_array_r = 0;
int parsing_array_sigma = 0;
int current_row, current_col;
int current_index;

int error_flag = 0;

/* Результаты преобразований */
int normal_n, normal_m;
double *normal_c;
double **normal_A;
double *normal_b;

int canon_n, canon_m;
double *canon_c;
double **canon_A;
double *canon_b;

void yyerror(const char *s);
int yylex(void);

void allocate_original();
void free_original();
void to_normal_form();
void to_canonical_form();
void output_json(const char *filename);
void write_matrix(FILE *f, double **mat, int rows, int cols);
void write_vector(FILE *f, double *vec, int len);

#line 122 "parser.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_TOKEN_N = 3,                    /* TOKEN_N  */
  YYSYMBOL_TOKEN_M = 4,                    /* TOKEN_M  */
  YYSYMBOL_TOKEN_DIRECTION = 5,            /* TOKEN_DIRECTION  */
  YYSYMBOL_TOKEN_C = 6,                    /* TOKEN_C  */
  YYSYMBOL_TOKEN_A = 7,                    /* TOKEN_A  */
  YYSYMBOL_TOKEN_B = 8,                    /* TOKEN_B  */
  YYSYMBOL_TOKEN_R = 9,                    /* TOKEN_R  */
  YYSYMBOL_TOKEN_SIGMA = 10,               /* TOKEN_SIGMA  */
  YYSYMBOL_NUMBER = 11,                    /* NUMBER  */
  YYSYMBOL_STRING = 12,                    /* STRING  */
  YYSYMBOL_13_ = 13,                       /* ','  */
  YYSYMBOL_14_ = 14,                       /* ':'  */
  YYSYMBOL_15_ = 15,                       /* '['  */
  YYSYMBOL_16_ = 16,                       /* ']'  */
  YYSYMBOL_17_ = 17,                       /* '{'  */
  YYSYMBOL_18_ = 18,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 19,                  /* $accept  */
  YYSYMBOL_input = 20,                     /* input  */
  YYSYMBOL_fields = 21,                    /* fields  */
  YYSYMBOL_field = 22,                     /* field  */
  YYSYMBOL_n_field = 23,                   /* n_field  */
  YYSYMBOL_m_field = 24,                   /* m_field  */
  YYSYMBOL_direction_field = 25,           /* direction_field  */
  YYSYMBOL_c_field = 26,                   /* c_field  */
  YYSYMBOL_27_1 = 27,                      /* $@1  */
  YYSYMBOL_vector_c = 28,                  /* vector_c  */
  YYSYMBOL_number_list_c = 29,             /* number_list_c  */
  YYSYMBOL_A_field = 30,                   /* A_field  */
  YYSYMBOL_31_2 = 31,                      /* $@2  */
  YYSYMBOL_matrix_A = 32,                  /* matrix_A  */
  YYSYMBOL_row_list_A = 33,                /* row_list_A  */
  YYSYMBOL_row_A = 34,                     /* row_A  */
  YYSYMBOL_number_list_A = 35,             /* number_list_A  */
  YYSYMBOL_b_field = 36,                   /* b_field  */
  YYSYMBOL_37_3 = 37,                      /* $@3  */
  YYSYMBOL_vector_b = 38,                  /* vector_b  */
  YYSYMBOL_number_list_b = 39,             /* number_list_b  */
  YYSYMBOL_r_field = 40,                   /* r_field  */
  YYSYMBOL_41_4 = 41,                      /* $@4  */
  YYSYMBOL_array_r = 42,                   /* array_r  */
  YYSYMBOL_string_list_r = 43,             /* string_list_r  */
  YYSYMBOL_sigma_field = 44,               /* sigma_field  */
  YYSYMBOL_45_5 = 45,                      /* $@5  */
  YYSYMBOL_array_sigma = 46,               /* array_sigma  */
  YYSYMBOL_string_list_sigma = 47          /* string_list_sigma  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  21
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   53

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  19
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  43
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  82

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   267


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    13,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    14,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    15,     2,    16,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    17,     2,    18,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    64,    64,    66,    66,    68,    68,    68,    68,    68,
      68,    68,    68,    70,    72,    74,    76,    76,    78,    80,
      82,    86,    86,    94,    96,    96,    98,   100,   102,   106,
     106,   108,   110,   112,   116,   116,   118,   120,   122,   126,
     126,   128,   130,   132
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "TOKEN_N", "TOKEN_M",
  "TOKEN_DIRECTION", "TOKEN_C", "TOKEN_A", "TOKEN_B", "TOKEN_R",
  "TOKEN_SIGMA", "NUMBER", "STRING", "','", "':'", "'['", "']'", "'{'",
  "'}'", "$accept", "input", "fields", "field", "n_field", "m_field",
  "direction_field", "c_field", "$@1", "vector_c", "number_list_c",
  "A_field", "$@2", "matrix_A", "row_list_A", "row_A", "number_list_A",
  "b_field", "$@3", "vector_b", "number_list_b", "r_field", "$@4",
  "array_r", "string_list_r", "sigma_field", "$@5", "array_sigma",
  "string_list_sigma", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-29)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
      -7,    -3,    22,     9,    11,    12,    13,    14,    15,    16,
      17,    -5,   -29,   -29,   -29,   -29,   -29,   -29,   -29,   -29,
     -29,   -29,    21,    23,    24,   -29,   -29,   -29,   -29,   -29,
      -3,   -29,   -29,   -29,   -29,    18,    20,    25,    26,    27,
     -29,    28,   -29,    29,   -29,    32,   -29,    33,   -29,    34,
     -29,   -29,    -4,    36,    -2,   -29,   -29,     2,   -29,     3,
     -29,     4,    37,   -29,   -29,     8,    29,   -29,    38,   -29,
      39,   -29,    40,   -29,   -29,    42,   -29,   -29,   -29,   -29,
     -29,   -29
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,     5,     6,     7,     8,     9,    10,    11,
      12,     1,     0,     0,     0,    16,    21,    29,    34,    39,
       0,     2,    13,    14,    15,     0,     0,     0,     0,     0,
       4,     0,    17,     0,    22,     0,    30,     0,    35,     0,
      40,    19,     0,     0,     0,    24,    32,     0,    37,     0,
      42,     0,     0,    18,    27,     0,     0,    23,     0,    31,
       0,    36,     0,    41,    20,     0,    26,    25,    33,    38,
      43,    28
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -29,   -29,   -29,     7,   -29,   -29,   -29,   -29,   -29,   -29,
     -29,   -29,   -29,   -29,   -29,   -28,   -29,   -29,   -29,   -29,
     -29,   -29,   -29,   -29,   -29,   -29,   -29,   -29,   -29
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,    11,    12,    13,    14,    15,    16,    35,    42,
      52,    17,    36,    44,    54,    55,    65,    18,    37,    46,
      57,    19,    38,    48,    59,    20,    39,    50,    61
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
       3,     4,     5,     6,     7,     8,     9,    10,    30,    62,
       1,    66,    63,    31,    67,    68,    70,    72,    69,    71,
      73,    75,    21,    22,    76,    23,    24,    25,    26,    27,
      28,    29,    32,    41,    33,    43,    34,    40,    77,    51,
      45,    47,    49,    56,    53,    58,    60,    64,    74,    78,
       0,    79,    80,    81
};

static const yytype_int8 yycheck[] =
{
       3,     4,     5,     6,     7,     8,     9,    10,    13,    13,
      17,    13,    16,    18,    16,    13,    13,    13,    16,    16,
      16,    13,     0,    14,    16,    14,    14,    14,    14,    14,
      14,    14,    11,    15,    11,    15,    12,    30,    66,    11,
      15,    15,    15,    11,    15,    12,    12,    11,    11,    11,
      -1,    12,    12,    11
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    17,    20,     3,     4,     5,     6,     7,     8,     9,
      10,    21,    22,    23,    24,    25,    26,    30,    36,    40,
      44,     0,    14,    14,    14,    14,    14,    14,    14,    14,
      13,    18,    11,    11,    12,    27,    31,    37,    41,    45,
      22,    15,    28,    15,    32,    15,    38,    15,    42,    15,
      46,    11,    29,    15,    33,    34,    11,    39,    12,    43,
      12,    47,    13,    16,    11,    35,    13,    16,    13,    16,
      13,    16,    13,    16,    11,    13,    16,    34,    11,    12,
      12,    11
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    19,    20,    21,    21,    22,    22,    22,    22,    22,
      22,    22,    22,    23,    24,    25,    27,    26,    28,    29,
      29,    31,    30,    32,    33,    33,    34,    35,    35,    37,
      36,    38,    39,    39,    41,    40,    42,    43,    43,    45,
      44,    46,    47,    47
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     1,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     0,     4,     3,     1,
       3,     0,     4,     3,     1,     3,     3,     1,     3,     0,
       4,     3,     1,     3,     0,     4,     3,     1,     3,     0,
       4,     3,     1,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 13: /* n_field: TOKEN_N ':' NUMBER  */
#line 70 "parser.y"
                            { n = (int)(yyvsp[0].num); }
#line 1200 "parser.tab.c"
    break;

  case 14: /* m_field: TOKEN_M ':' NUMBER  */
#line 72 "parser.y"
                            { m = (int)(yyvsp[0].num); allocate_original(); }
#line 1206 "parser.tab.c"
    break;

  case 15: /* direction_field: TOKEN_DIRECTION ':' STRING  */
#line 74 "parser.y"
                                            { strcpy(direction, (yyvsp[0].string)); free((yyvsp[0].string)); }
#line 1212 "parser.tab.c"
    break;

  case 16: /* $@1: %empty  */
#line 76 "parser.y"
                     { parsing_vector_c = 1; current_index = 0; }
#line 1218 "parser.tab.c"
    break;

  case 17: /* c_field: TOKEN_C ':' $@1 vector_c  */
#line 76 "parser.y"
                                                                           { parsing_vector_c = 0; }
#line 1224 "parser.tab.c"
    break;

  case 19: /* number_list_c: NUMBER  */
#line 80 "parser.y"
                      {
    if (parsing_vector_c) c[current_index++] = (yyvsp[0].num);
}
#line 1232 "parser.tab.c"
    break;

  case 20: /* number_list_c: number_list_c ',' NUMBER  */
#line 82 "parser.y"
                             {
    if (parsing_vector_c) c[current_index++] = (yyvsp[0].num);
}
#line 1240 "parser.tab.c"
    break;

  case 21: /* $@2: %empty  */
#line 86 "parser.y"
                     {
    parsing_matrix_A = 1;
    current_row = 0;
    current_col = 0;
}
#line 1250 "parser.tab.c"
    break;

  case 22: /* A_field: TOKEN_A ':' $@2 matrix_A  */
#line 90 "parser.y"
           {
    parsing_matrix_A = 0;
}
#line 1258 "parser.tab.c"
    break;

  case 26: /* row_A: '[' number_list_A ']'  */
#line 98 "parser.y"
                             { current_row++; current_col = 0; }
#line 1264 "parser.tab.c"
    break;

  case 27: /* number_list_A: NUMBER  */
#line 100 "parser.y"
                      {
    if (parsing_matrix_A) A[current_row][current_col++] = (yyvsp[0].num);
}
#line 1272 "parser.tab.c"
    break;

  case 28: /* number_list_A: number_list_A ',' NUMBER  */
#line 102 "parser.y"
                             {
    if (parsing_matrix_A) A[current_row][current_col++] = (yyvsp[0].num);
}
#line 1280 "parser.tab.c"
    break;

  case 29: /* $@3: %empty  */
#line 106 "parser.y"
                     { parsing_vector_b = 1; current_index = 0; }
#line 1286 "parser.tab.c"
    break;

  case 30: /* b_field: TOKEN_B ':' $@3 vector_b  */
#line 106 "parser.y"
                                                                           { parsing_vector_b = 0; }
#line 1292 "parser.tab.c"
    break;

  case 32: /* number_list_b: NUMBER  */
#line 110 "parser.y"
                      {
    if (parsing_vector_b) b[current_index++] = (yyvsp[0].num);
}
#line 1300 "parser.tab.c"
    break;

  case 33: /* number_list_b: number_list_b ',' NUMBER  */
#line 112 "parser.y"
                             {
    if (parsing_vector_b) b[current_index++] = (yyvsp[0].num);
}
#line 1308 "parser.tab.c"
    break;

  case 34: /* $@4: %empty  */
#line 116 "parser.y"
                     { parsing_array_r = 1; current_index = 0; }
#line 1314 "parser.tab.c"
    break;

  case 35: /* r_field: TOKEN_R ':' $@4 array_r  */
#line 116 "parser.y"
                                                                         { parsing_array_r = 0; }
#line 1320 "parser.tab.c"
    break;

  case 37: /* string_list_r: STRING  */
#line 120 "parser.y"
                      {
    if (parsing_array_r) r[current_index++] = (yyvsp[0].string);
}
#line 1328 "parser.tab.c"
    break;

  case 38: /* string_list_r: string_list_r ',' STRING  */
#line 122 "parser.y"
                             {
    if (parsing_array_r) r[current_index++] = (yyvsp[0].string);
}
#line 1336 "parser.tab.c"
    break;

  case 39: /* $@5: %empty  */
#line 126 "parser.y"
                             { parsing_array_sigma = 1; current_index = 0; }
#line 1342 "parser.tab.c"
    break;

  case 40: /* sigma_field: TOKEN_SIGMA ':' $@5 array_sigma  */
#line 126 "parser.y"
                                                                                         { parsing_array_sigma = 0; }
#line 1348 "parser.tab.c"
    break;

  case 42: /* string_list_sigma: STRING  */
#line 130 "parser.y"
                          {
    if (parsing_array_sigma) sigma[current_index++] = (yyvsp[0].string);
}
#line 1356 "parser.tab.c"
    break;

  case 43: /* string_list_sigma: string_list_sigma ',' STRING  */
#line 132 "parser.y"
                                 {
    if (parsing_array_sigma) sigma[current_index++] = (yyvsp[0].string);
}
#line 1364 "parser.tab.c"
    break;


#line 1368 "parser.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 136 "parser.y"


void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    error_flag = 1;
}

void allocate_original() {
    c = (double*)malloc(n * sizeof(double));
    b = (double*)malloc(m * sizeof(double));
    A = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++)
        A[i] = (double*)malloc(n * sizeof(double));
    r = (char**)malloc(m * sizeof(char*));
    sigma = (char**)malloc(n * sizeof(char*));
    for (int i = 0; i < m; i++) r[i] = NULL;
    for (int i = 0; i < n; i++) sigma[i] = NULL;
}

void free_original() {
    free(c);
    free(b);
    for (int i = 0; i < m; i++) free(A[i]);
    free(A);
    for (int i = 0; i < m; i++) if (r[i]) free(r[i]);
    free(r);
    for (int i = 0; i < n; i++) if (sigma[i]) free(sigma[i]);
    free(sigma);
}

/* Приведение к нормальной форме */
void to_normal_form() {
    double coeff_mult = (strcmp(direction, "min") == 0) ? -1.0 : 1.0;

    /* ---- Приведение ограничений к виду "≤" ---- */
    int temp_m = m;
    for (int i = 0; i < m; i++)
        if (strcmp(r[i], "=") == 0) temp_m++;

    double **A_temp = (double**)malloc(temp_m * sizeof(double*));
    double *b_temp = (double*)malloc(temp_m * sizeof(double));
    int row_idx = 0;
    for (int i = 0; i < m; i++) {
        if (strcmp(r[i], "<=") == 0) {
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = A[i][j];
            b_temp[row_idx] = b[i];
            row_idx++;
        } else if (strcmp(r[i], ">=") == 0) {
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = -A[i][j];
            b_temp[row_idx] = -b[i];
            row_idx++;
        } else if (strcmp(r[i], "=") == 0) {
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = A[i][j];
            b_temp[row_idx] = b[i];
            row_idx++;
            A_temp[row_idx] = (double*)malloc(n * sizeof(double));
            for (int j = 0; j < n; j++) A_temp[row_idx][j] = -A[i][j];
            b_temp[row_idx] = -b[i];
            row_idx++;
        }
    }

    /* ---- Обработка переменных ---- */
    int new_n = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) new_n++;
        else if (strcmp(sigma[j], "<=0") == 0) new_n++;
        else if (strcmp(sigma[j], "free") == 0) new_n += 2;
    }

    normal_n = new_n;
    normal_m = temp_m;
    normal_c = (double*)malloc(normal_n * sizeof(double));
    normal_A = (double**)malloc(normal_m * sizeof(double*));
    for (int i = 0; i < normal_m; i++)
        normal_A[i] = (double*)malloc(normal_n * sizeof(double));
    normal_b = (double*)malloc(normal_m * sizeof(double));
    for (int i = 0; i < normal_m; i++) normal_b[i] = b_temp[i];

    int new_col = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) {
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = A_temp[i][j];
            normal_c[new_col] = coeff_mult * c[j];
            new_col++;
        } else if (strcmp(sigma[j], "<=0") == 0) {
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = -A_temp[i][j];
            normal_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        } else if (strcmp(sigma[j], "free") == 0) {
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = A_temp[i][j];
            normal_c[new_col] = coeff_mult * c[j];
            new_col++;
            for (int i = 0; i < normal_m; i++)
                normal_A[i][new_col] = -A_temp[i][j];
            normal_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        }
    }

    /* Освобождение временных данных */
    for (int i = 0; i < temp_m; i++) free(A_temp[i]);
    free(A_temp);
    free(b_temp);
}

/* Приведение к канонической форме */
void to_canonical_form() {
    double coeff_mult = (strcmp(direction, "min") == 0) ? -1.0 : 1.0;

    /* ---- Добавление slack/surplus переменных для неравенств ---- */
    int num_slack = 0;
    for (int i = 0; i < m; i++)
        if (strcmp(r[i], "<=") == 0 || strcmp(r[i], ">=") == 0) num_slack++;

    int temp_n = n + num_slack;
    double **A_temp = (double**)malloc(m * sizeof(double*));
    double *b_temp = (double*)malloc(m * sizeof(double));
    for (int i = 0; i < m; i++) {
        A_temp[i] = (double*)malloc(temp_n * sizeof(double));
        for (int j = 0; j < n; j++) A_temp[i][j] = A[i][j];
    }
    int slack_idx = n;
    for (int i = 0; i < m; i++) {
        if (strcmp(r[i], "<=") == 0) {
            for (int k = 0; k < m; k++)
                A_temp[k][slack_idx] = (k == i) ? 1.0 : 0.0;
            slack_idx++;
        } else if (strcmp(r[i], ">=") == 0) {
            for (int k = 0; k < m; k++)
                A_temp[k][slack_idx] = (k == i) ? -1.0 : 0.0;
            slack_idx++;
        }
    }
    for (int i = 0; i < m; i++) b_temp[i] = b[i];

    /* ---- Обработка переменных (исходные + slack) ---- */
    int new_n = 0;
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) new_n++;
        else if (strcmp(sigma[j], "<=0") == 0) new_n++;
        else if (strcmp(sigma[j], "free") == 0) new_n += 2;
    }
    new_n += num_slack;

    canon_n = new_n;
    canon_m = m;
    canon_c = (double*)malloc(canon_n * sizeof(double));
    canon_A = (double**)malloc(canon_m * sizeof(double*));
    for (int i = 0; i < canon_m; i++)
        canon_A[i] = (double*)malloc(canon_n * sizeof(double));
    canon_b = (double*)malloc(canon_m * sizeof(double));
    for (int i = 0; i < canon_m; i++) canon_b[i] = b_temp[i];

    int new_col = 0;
    /* Исходные переменные */
    for (int j = 0; j < n; j++) {
        if (strcmp(sigma[j], ">=0") == 0) {
            for (int i = 0; i < canon_m; i++)
                canon_A[i][new_col] = A_temp[i][j];
            canon_c[new_col] = coeff_mult * c[j];
            new_col++;
        } else if (strcmp(sigma[j], "<=0") == 0) {
            for (int i = 0; i < canon_m; i++)
                canon_A[i][new_col] = -A_temp[i][j];
            canon_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        } else if (strcmp(sigma[j], "free") == 0) {
            for (int i = 0; i < canon_m; i++)
                canon_A[i][new_col] = A_temp[i][j];
            canon_c[new_col] = coeff_mult * c[j];
            new_col++;
            for (int i = 0; i < canon_m; i++)
                canon_A[i][new_col] = -A_temp[i][j];
            canon_c[new_col] = coeff_mult * (-c[j]);
            new_col++;
        }
    }
    /* Slack переменные */
    for (int s = 0; s < num_slack; s++) {
        int col = n + s;
        for (int i = 0; i < canon_m; i++)
            canon_A[i][new_col] = A_temp[i][col];
        canon_c[new_col] = 0.0;
        new_col++;
    }

    /* Освобождение временных данных */
    for (int i = 0; i < m; i++) free(A_temp[i]);
    free(A_temp);
    free(b_temp);
}

void write_matrix(FILE *f, double **mat, int rows, int cols) {
    fprintf(f, "[");
    for (int i = 0; i < rows; i++) {
        fprintf(f, "[");
        for (int j = 0; j < cols; j++) {
            fprintf(f, "%.6f", mat[i][j]);
            if (j < cols-1) fprintf(f, ", ");
        }
        fprintf(f, "]");
        if (i < rows-1) fprintf(f, ", ");
    }
    fprintf(f, "]");
}

void write_vector(FILE *f, double *vec, int len) {
    fprintf(f, "[");
    for (int i = 0; i < len; i++) {
        fprintf(f, "%.6f", vec[i]);
        if (i < len-1) fprintf(f, ", ");
    }
    fprintf(f, "]");
}

void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Cannot open output file %s\n", filename);
        return;
    }
    fprintf(f, "{\n");
    fprintf(f, "  \"normal_form\": {\n");
    fprintf(f, "    \"direction\": \"max\",\n");
    fprintf(f, "    \"c\": ");
    write_vector(f, normal_c, normal_n);
    fprintf(f, ",\n");
    fprintf(f, "    \"A\": ");
    write_matrix(f, normal_A, normal_m, normal_n);
    fprintf(f, ",\n");
    fprintf(f, "    \"b\": ");
    write_vector(f, normal_b, normal_m);
    fprintf(f, "\n");
    fprintf(f, "  },\n");
    fprintf(f, "  \"canonical_form\": {\n");
    fprintf(f, "    \"direction\": \"max\",\n");
    fprintf(f, "    \"c\": ");
    write_vector(f, canon_c, canon_n);
    fprintf(f, ",\n");
    fprintf(f, "    \"A\": ");
    write_matrix(f, canon_A, canon_m, canon_n);
    fprintf(f, ",\n");
    fprintf(f, "    \"b\": ");
    write_vector(f, canon_b, canon_m);
    fprintf(f, "\n");
    fprintf(f, "  }\n");
    fprintf(f, "}\n");
    fclose(f);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s input.json\n", argv[0]);
        return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Cannot open input file %s\n", argv[1]);
        return 1;
    }
    yyin = file;
    yyparse();
    fclose(file);
    if (error_flag) {
        fprintf(stderr, "Parsing failed.\n");
        return 1;
    }
    to_normal_form();
    to_canonical_form();
    output_json("output.json");

    free_original();
    free(normal_c);
    for (int i = 0; i < normal_m; i++) free(normal_A[i]);
    free(normal_A);
    free(normal_b);
    free(canon_c);
    for (int i = 0; i < canon_m; i++) free(canon_A[i]);
    free(canon_A);
    free(canon_b);

    return 0;
}
