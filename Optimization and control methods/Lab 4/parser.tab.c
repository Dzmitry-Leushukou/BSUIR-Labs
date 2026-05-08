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
#line 2 "parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern FILE *yyin;

int n, m;
double *c;
double **A;
double *b;

int parsing_matrix = 0;
int parsing_vector = 0;
int current_row, current_col;
int current_index;
int error_flag = 0;

double *x_opt;
double obj_value;
int infeasible = 0;
int *final_basis;
int final_basis_size;

void yyerror(const char *s);
int yylex(void);

void allocate_data();
void free_data();
int solve_two_phase();
int invert_matrix(double **mat, double **inv, int size);
void simplex_main_phase(int phase_n, int phase_m, double *phase_c, double **phase_A, double *phase_b, double *phase_x, int *phase_basis, int *phase_basis_size, int *unbounded);
void output_json(const char *filename);

#line 107 "parser.tab.c"

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
  YYSYMBOL_TOKEN_C = 5,                    /* TOKEN_C  */
  YYSYMBOL_TOKEN_A = 6,                    /* TOKEN_A  */
  YYSYMBOL_TOKEN_B = 7,                    /* TOKEN_B  */
  YYSYMBOL_NUMBER = 8,                     /* NUMBER  */
  YYSYMBOL_9_ = 9,                         /* ','  */
  YYSYMBOL_10_ = 10,                       /* ':'  */
  YYSYMBOL_11_ = 11,                       /* '['  */
  YYSYMBOL_12_ = 12,                       /* ']'  */
  YYSYMBOL_13_ = 13,                       /* '{'  */
  YYSYMBOL_14_ = 14,                       /* '}'  */
  YYSYMBOL_YYACCEPT = 15,                  /* $accept  */
  YYSYMBOL_input = 16,                     /* input  */
  YYSYMBOL_fields = 17,                    /* fields  */
  YYSYMBOL_field = 18,                     /* field  */
  YYSYMBOL_n_field = 19,                   /* n_field  */
  YYSYMBOL_m_field = 20,                   /* m_field  */
  YYSYMBOL_c_field = 21,                   /* c_field  */
  YYSYMBOL_22_1 = 22,                      /* $@1  */
  YYSYMBOL_vector_c = 23,                  /* vector_c  */
  YYSYMBOL_number_list_c = 24,             /* number_list_c  */
  YYSYMBOL_A_field = 25,                   /* A_field  */
  YYSYMBOL_26_2 = 26,                      /* $@2  */
  YYSYMBOL_matrix_A = 27,                  /* matrix_A  */
  YYSYMBOL_row_list_A = 28,                /* row_list_A  */
  YYSYMBOL_row_A = 29,                     /* row_A  */
  YYSYMBOL_number_list_A = 30,             /* number_list_A  */
  YYSYMBOL_b_field = 31,                   /* b_field  */
  YYSYMBOL_32_3 = 32,                      /* $@3  */
  YYSYMBOL_vector_b = 33,                  /* vector_b  */
  YYSYMBOL_number_list_b = 34              /* number_list_b  */
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
#define YYFINAL  15
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   41

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  15
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  20
/* YYNRULES -- Number of rules.  */
#define YYNRULES  29
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  56

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   263


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
       2,     2,     2,     2,     9,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    10,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    11,     2,    12,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    13,     2,    14,     2,     2,     2,     2,
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
       5,     6,     7,     8
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int8 yyrline[] =
{
       0,    48,    48,    50,    50,    52,    52,    52,    52,    52,
      54,    56,    58,    58,    60,    62,    64,    68,    68,    76,
      78,    78,    80,    82,    84,    88,    88,    90,    92,    94
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
  "TOKEN_C", "TOKEN_A", "TOKEN_B", "NUMBER", "','", "':'", "'['", "']'",
  "'{'", "'}'", "$accept", "input", "fields", "field", "n_field",
  "m_field", "c_field", "$@1", "vector_c", "number_list_c", "A_field",
  "$@2", "matrix_A", "row_list_A", "row_A", "number_list_A", "b_field",
  "$@3", "vector_b", "number_list_b", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-22)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -11,     3,    12,    -7,     7,     9,    10,    11,    -9,   -22,
     -22,   -22,   -22,   -22,   -22,   -22,    14,    15,   -22,   -22,
     -22,     3,   -22,   -22,   -22,    13,    16,    17,   -22,    18,
     -22,    19,   -22,    21,   -22,   -22,    -8,    23,     2,   -22,
     -22,     4,    24,   -22,   -22,     6,    19,   -22,    25,   -22,
     -22,    26,   -22,   -22,   -22,   -22
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     0,     0,     0,     0,     0,     0,     0,     0,     3,
       5,     6,     7,     8,     9,     1,     0,     0,    12,    17,
      25,     0,     2,    10,    11,     0,     0,     0,     4,     0,
      13,     0,    18,     0,    26,    15,     0,     0,     0,    20,
      28,     0,     0,    14,    23,     0,     0,    19,     0,    27,
      16,     0,    22,    21,    29,    24
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -22,   -22,   -22,    20,   -22,   -22,   -22,   -22,   -22,   -22,
     -22,   -22,   -22,   -22,   -21,   -22,   -22,   -22,   -22,   -22
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     2,     8,     9,    10,    11,    12,    25,    30,    36,
      13,    26,    32,    38,    39,    45,    14,    27,    34,    41
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      21,    42,     1,    16,    43,    22,     3,     4,     5,     6,
       7,    46,    15,    48,    47,    51,    49,    17,    52,    18,
      19,    20,    23,    24,    29,    53,    35,    31,    33,    40,
      37,    44,    50,    54,    55,     0,     0,     0,     0,     0,
       0,    28
};

static const yytype_int8 yycheck[] =
{
       9,     9,    13,    10,    12,    14,     3,     4,     5,     6,
       7,     9,     0,     9,    12,     9,    12,    10,    12,    10,
      10,    10,     8,     8,    11,    46,     8,    11,    11,     8,
      11,     8,     8,     8,     8,    -1,    -1,    -1,    -1,    -1,
      -1,    21
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    13,    16,     3,     4,     5,     6,     7,    17,    18,
      19,    20,    21,    25,    31,     0,    10,    10,    10,    10,
      10,     9,    14,     8,     8,    22,    26,    32,    18,    11,
      23,    11,    27,    11,    33,     8,    24,    11,    28,    29,
       8,    34,     9,    12,     8,    30,     9,    12,     9,    12,
       8,     9,    12,    29,     8,     8
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    15,    16,    17,    17,    18,    18,    18,    18,    18,
      19,    20,    22,    21,    23,    24,    24,    26,    25,    27,
      28,    28,    29,    30,    30,    32,    31,    33,    34,    34
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     3,     1,     3,     1,     1,     1,     1,     1,
       3,     3,     0,     4,     3,     1,     3,     0,     4,     3,
       1,     3,     3,     1,     3,     0,     4,     3,     1,     3
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
  case 10: /* n_field: TOKEN_N ':' NUMBER  */
#line 54 "parser.y"
                            { n = (int)(yyvsp[0].num); }
#line 1150 "parser.tab.c"
    break;

  case 11: /* m_field: TOKEN_M ':' NUMBER  */
#line 56 "parser.y"
                            { m = (int)(yyvsp[0].num); allocate_data(); }
#line 1156 "parser.tab.c"
    break;

  case 12: /* $@1: %empty  */
#line 58 "parser.y"
                     { parsing_vector = 1; current_index = 0; }
#line 1162 "parser.tab.c"
    break;

  case 13: /* c_field: TOKEN_C ':' $@1 vector_c  */
#line 58 "parser.y"
                                                                         { parsing_vector = 0; }
#line 1168 "parser.tab.c"
    break;

  case 15: /* number_list_c: NUMBER  */
#line 62 "parser.y"
                      {
    if (parsing_vector) c[current_index++] = (yyvsp[0].num);
}
#line 1176 "parser.tab.c"
    break;

  case 16: /* number_list_c: number_list_c ',' NUMBER  */
#line 64 "parser.y"
                             {
    if (parsing_vector) c[current_index++] = (yyvsp[0].num);
}
#line 1184 "parser.tab.c"
    break;

  case 17: /* $@2: %empty  */
#line 68 "parser.y"
                     {
    parsing_matrix = 1;
    current_row = 0;
    current_col = 0;
}
#line 1194 "parser.tab.c"
    break;

  case 18: /* A_field: TOKEN_A ':' $@2 matrix_A  */
#line 72 "parser.y"
           {
    parsing_matrix = 0;
}
#line 1202 "parser.tab.c"
    break;

  case 22: /* row_A: '[' number_list_A ']'  */
#line 80 "parser.y"
                             { current_row++; current_col = 0; }
#line 1208 "parser.tab.c"
    break;

  case 23: /* number_list_A: NUMBER  */
#line 82 "parser.y"
                      {
    if (parsing_matrix) A[current_row][current_col++] = (yyvsp[0].num);
}
#line 1216 "parser.tab.c"
    break;

  case 24: /* number_list_A: number_list_A ',' NUMBER  */
#line 84 "parser.y"
                             {
    if (parsing_matrix) A[current_row][current_col++] = (yyvsp[0].num);
}
#line 1224 "parser.tab.c"
    break;

  case 25: /* $@3: %empty  */
#line 88 "parser.y"
                     { parsing_vector = 1; current_index = 0; }
#line 1230 "parser.tab.c"
    break;

  case 26: /* b_field: TOKEN_B ':' $@3 vector_b  */
#line 88 "parser.y"
                                                                         { parsing_vector = 0; }
#line 1236 "parser.tab.c"
    break;

  case 28: /* number_list_b: NUMBER  */
#line 92 "parser.y"
                      {
    if (parsing_vector) b[current_index++] = (yyvsp[0].num);
}
#line 1244 "parser.tab.c"
    break;

  case 29: /* number_list_b: number_list_b ',' NUMBER  */
#line 94 "parser.y"
                             {
    if (parsing_vector) b[current_index++] = (yyvsp[0].num);
}
#line 1252 "parser.tab.c"
    break;


#line 1256 "parser.tab.c"

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

#line 98 "parser.y"


void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
    error_flag = 1;
}

void allocate_data() {
    c = (double*)malloc(n * sizeof(double));
    b = (double*)malloc(m * sizeof(double));
    A = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++)
        A[i] = (double*)malloc(n * sizeof(double));
    x_opt = (double*)malloc(n * sizeof(double));
    final_basis = (int*)malloc(m * sizeof(int));
}

void free_data() {
    free(c);
    free(b);
    for (int i = 0; i < m; i++) free(A[i]);
    free(A);
    free(x_opt);
    free(final_basis);
}

int invert_matrix(double **mat, double **inv, int size) {
    double **aug = (double**)malloc(size * sizeof(double*));
    for (int i = 0; i < size; i++) {
        aug[i] = (double*)malloc(2 * size * sizeof(double));
        for (int j = 0; j < size; j++) aug[i][j] = mat[i][j];
        for (int j = size; j < 2 * size; j++) aug[i][j] = (j - size == i) ? 1.0 : 0.0;
    }
    
    for (int col = 0; col < size; col++) {
        int pivot = -1;
        for (int i = col; i < size; i++) {
            if (fabs(aug[i][col]) > 1e-12) { pivot = i; break; }
        }
        if (pivot == -1) {
            for (int i = 0; i < size; i++) free(aug[i]);
            free(aug);
            return 0;
        }
        if (pivot != col) {
            double *tmp = aug[col];
            aug[col] = aug[pivot];
            aug[pivot] = tmp;
        }
        double piv_val = aug[col][col];
        for (int j = 0; j < 2 * size; j++) aug[col][j] /= piv_val;
        for (int i = 0; i < size; i++) {
            if (i != col) {
                double factor = aug[i][col];
                for (int j = 0; j < 2 * size; j++)
                    aug[i][j] -= factor * aug[col][j];
            }
        }
    }
    
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            inv[i][j] = aug[i][size + j];
    
    for (int i = 0; i < size; i++) free(aug[i]);
    free(aug);
    return 1;
}

void simplex_main_phase(int phase_n, int phase_m, double *phase_c, double **phase_A, double *phase_b, double *phase_x, int *phase_basis, int *phase_basis_size, int *unbounded) {
    double **A_B = (double**)malloc(phase_m * sizeof(double*));
    double **inv_A_B = (double**)malloc(phase_m * sizeof(double*));
    for (int i = 0; i < phase_m; i++) {
        A_B[i] = (double*)malloc(phase_m * sizeof(double));
        inv_A_B[i] = (double*)malloc(phase_m * sizeof(double));
    }
    
    double *u = (double*)malloc(phase_m * sizeof(double));
    double *delta = (double*)malloc(phase_n * sizeof(double));
    double *z = (double*)malloc(phase_m * sizeof(double));
    double *theta = (double*)malloc(phase_m * sizeof(double));
    double *xB = (double*)malloc(phase_m * sizeof(double));
    
    for (int i = 0; i < phase_m; i++) {
        xB[i] = phase_x[phase_basis[i] - 1];
    }
    
    int iteration = 0;
    const int max_iter = 1000;
    *unbounded = 0;
    
    while (iteration++ < max_iter) {
        for (int i = 0; i < phase_m; i++)
            for (int j = 0; j < phase_m; j++)
                A_B[i][j] = phase_A[i][phase_basis[j] - 1];
        
        if (!invert_matrix(A_B, inv_A_B, phase_m)) {
            fprintf(stderr, "Basis matrix singular at iteration %d\n", iteration);
            exit(1);
        }
        
        double *cB = (double*)malloc(phase_m * sizeof(double));
        for (int i = 0; i < phase_m; i++) cB[i] = phase_c[phase_basis[i] - 1];
        
        for (int i = 0; i < phase_m; i++) {
            u[i] = 0.0;
            for (int j = 0; j < phase_m; j++)
                u[i] += cB[j] * inv_A_B[j][i];
        }
        
        for (int j = 0; j < phase_n; j++) {
            double uA = 0.0;
            for (int i = 0; i < phase_m; i++) uA += u[i] * phase_A[i][j];
            delta[j] = phase_c[j] - uA;
        }
        
        int optimal = 1;
        for (int j = 0; j < phase_n; j++) {
            if (delta[j] > 1e-9) { optimal = 0; break; }
        }
        
        if (optimal) {
            free(cB);
            break;
        }
        
        int j0 = -1;
        for (int j = 0; j < phase_n; j++) {
            if (delta[j] > 1e-9) { j0 = j; break; }
        }
        
        for (int i = 0; i < phase_m; i++) {
            z[i] = 0.0;
            for (int k = 0; k < phase_m; k++)
                z[i] += inv_A_B[i][k] * phase_A[k][j0];
        }
        
        double theta0 = INFINITY;
        int k = -1;
        for (int i = 0; i < phase_m; i++) {
            if (z[i] > 1e-12) {
                theta[i] = xB[i] / z[i];
                if (theta[i] < theta0) {
                    theta0 = theta[i];
                    k = i;
                }
            } else {
                theta[i] = INFINITY;
            }
        }
        
        if (theta0 == INFINITY) {
            *unbounded = 1;
            free(cB);
            break;
        }
        
        phase_basis[k] = j0 + 1;
        
        double new_xB_j0 = theta0;
        for (int i = 0; i < phase_m; i++) {
            if (i != k) {
                xB[i] = xB[i] - theta0 * z[i];
                if (xB[i] < 0 && xB[i] > -1e-9) xB[i] = 0.0;
            }
        }
        xB[k] = new_xB_j0;
        
        for (int j = 0; j < phase_n; j++) phase_x[j] = 0.0;
        for (int i = 0; i < phase_m; i++) phase_x[phase_basis[i] - 1] = xB[i];
        
        free(cB);
    }
    
    for (int i = 0; i < phase_m; i++) {
        free(A_B[i]);
        free(inv_A_B[i]);
    }
    free(A_B);
    free(inv_A_B);
    free(u);
    free(delta);
    free(z);
    free(theta);
    free(xB);
}

int solve_two_phase() {
    for (int i = 0; i < m; i++) {
        if (b[i] < 0) {
            b[i] = -b[i];
            for (int j = 0; j < n; j++)
                A[i][j] = -A[i][j];
        }
    }
    
    int phase_n = n + m;
    double *phase_c = (double*)malloc(phase_n * sizeof(double));
    double **phase_A = (double**)malloc(m * sizeof(double*));
    for (int i = 0; i < m; i++) {
        phase_A[i] = (double*)malloc(phase_n * sizeof(double));
        for (int j = 0; j < n; j++)
            phase_A[i][j] = A[i][j];
        for (int j = 0; j < m; j++)
            phase_A[i][n + j] = (i == j) ? 1.0 : 0.0;
    }
    
    for (int j = 0; j < n; j++)
        phase_c[j] = 0.0;
    for (int j = 0; j < m; j++)
        phase_c[n + j] = -1.0;
    
    double *phase_x = (double*)malloc(phase_n * sizeof(double));
    for (int j = 0; j < n; j++) phase_x[j] = 0.0;
    for (int i = 0; i < m; i++) phase_x[n + i] = b[i];
    
    int *phase_basis = (int*)malloc(m * sizeof(int));
    for (int i = 0; i < m; i++)
        phase_basis[i] = n + i + 1;
    
    int phase_basis_size = m;
    int unbounded_flag;
    
    simplex_main_phase(phase_n, m, phase_c, phase_A, b, phase_x, phase_basis, &phase_basis_size, &unbounded_flag);
    
    double artificial_sum = 0.0;
    for (int i = 0; i < m; i++)
        artificial_sum += phase_x[n + i];
    
    if (artificial_sum > 1e-9) {
        infeasible = 1;
        free(phase_c);
        for (int i = 0; i < m; i++) free(phase_A[i]);
        free(phase_A);
        free(phase_x);
        free(phase_basis);
        return 0;
    }
    
    for (int j = 0; j < n; j++)
        x_opt[j] = phase_x[j];
    
    int remaining = 0;
    for (int i = 0; i < m; i++) {
        if (phase_basis[i] <= n) {
            final_basis[remaining++] = phase_basis[i];
        }
    }
    
    if (remaining == m) {
        final_basis_size = m;
        free(phase_c);
        for (int i = 0; i < m; i++) free(phase_A[i]);
        free(phase_A);
        free(phase_x);
        free(phase_basis);
        return 1;
    }
    
    int rows_to_keep = m;
    int *keep_row = (int*)malloc(m * sizeof(int));
    for (int i = 0; i < m; i++) keep_row[i] = 1;
    
    while (1) {
        int artificial_found = -1;
        for (int i = 0; i < phase_basis_size; i++) {
            if (phase_basis[i] > n) {
                artificial_found = i;
                break;
            }
        }
        
        if (artificial_found == -1) break;
        
        int i_idx = phase_basis[artificial_found] - n - 1;
        
        double **A_B_curr = (double**)malloc(phase_basis_size * sizeof(double*));
        double **inv_A_B_curr = (double**)malloc(phase_basis_size * sizeof(double*));
        for (int ii = 0; ii < phase_basis_size; ii++) {
            A_B_curr[ii] = (double*)malloc(phase_basis_size * sizeof(double));
            inv_A_B_curr[ii] = (double*)malloc(phase_basis_size * sizeof(double));
            for (int jj = 0; jj < phase_basis_size; jj++) {
                A_B_curr[ii][jj] = phase_A[ii][phase_basis[jj] - 1];
            }
        }
        
        invert_matrix(A_B_curr, inv_A_B_curr, phase_basis_size);
        
        int replacement_found = 0;
        for (int j = 0; j < n; j++) {
            int is_basic = 0;
            for (int ii = 0; ii < phase_basis_size; ii++) {
                if (phase_basis[ii] == j + 1) {
                    is_basic = 1;
                    break;
                }
            }
            if (is_basic) continue;
            
            double *lvec = (double*)malloc(phase_basis_size * sizeof(double));
            for (int ii = 0; ii < phase_basis_size; ii++) {
                lvec[ii] = 0.0;
                for (int kk = 0; kk < phase_basis_size; kk++) {
                    lvec[ii] += inv_A_B_curr[ii][kk] * phase_A[kk][j];
                }
            }
            
            if (fabs(lvec[artificial_found]) > 1e-12) {
                phase_basis[artificial_found] = j + 1;
                replacement_found = 1;
                free(lvec);
                break;
            }
            free(lvec);
        }
        
        for (int ii = 0; ii < phase_basis_size; ii++) {
            free(A_B_curr[ii]);
            free(inv_A_B_curr[ii]);
        }
        free(A_B_curr);
        free(inv_A_B_curr);
        
        if (!replacement_found) {
            keep_row[i_idx] = 0;
            rows_to_keep--;
            for (int k = artificial_found; k < phase_basis_size - 1; k++) {
                phase_basis[k] = phase_basis[k + 1];
            }
            phase_basis_size--;
        }
    }
    
    final_basis_size = 0;
    for (int i = 0; i < phase_basis_size; i++) {
        if (phase_basis[i] <= n) {
            final_basis[final_basis_size++] = phase_basis[i];
        }
    }
    
    if (rows_to_keep < m) {
        double **new_A = (double**)malloc(rows_to_keep * sizeof(double*));
        double *new_b = (double*)malloc(rows_to_keep * sizeof(double));
        int new_row = 0;
        for (int i = 0; i < m; i++) {
            if (keep_row[i]) {
                new_A[new_row] = (double*)malloc(n * sizeof(double));
                for (int j = 0; j < n; j++)
                    new_A[new_row][j] = A[i][j];
                new_b[new_row] = b[i];
                new_row++;
            }
        }
        
        for (int i = 0; i < m; i++) free(A[i]);
        free(A);
        free(b);
        
        m = rows_to_keep;
        A = new_A;
        b = new_b;
    }
    
    free(keep_row);
    free(phase_c);
    for (int i = 0; i < m; i++) free(phase_A[i]);
    free(phase_A);
    free(phase_x);
    free(phase_basis);
    
    return 1;
}

void output_json(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("Cannot open output file"); return; }
    
    fprintf(f, "{\n");
    if (infeasible) {
        fprintf(f, "  \"status\": \"infeasible\",\n");
        fprintf(f, "  \"message\": \"The problem has no feasible solutions\"\n");
    } else {
        obj_value = 0.0;
        for (int j = 0; j < n; j++)
            obj_value += c[j] * x_opt[j];
        
        fprintf(f, "  \"status\": \"optimal\",\n");
        fprintf(f, "  \"objective\": %.10f,\n", obj_value);
        fprintf(f, "  \"x\": [");
        for (int j = 0; j < n; j++) {
            fprintf(f, "%.10f", x_opt[j]);
            if (j < n - 1) fprintf(f, ", ");
        }
        fprintf(f, "],\n");
        fprintf(f, "  \"basis\": [");
        for (int i = 0; i < final_basis_size; i++) {
            fprintf(f, "%d", final_basis[i]);
            if (i < final_basis_size - 1) fprintf(f, ", ");
        }
        fprintf(f, "]\n");
    }
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
    
    solve_two_phase();
    output_json("output.json");
    free_data();
    return 0;
}
