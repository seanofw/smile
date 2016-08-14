#ifndef __SMILE_ENV_KNOWNSYMBOLS_H__
#define __SMILE_ENV_KNOWNSYMBOLS_H__

#ifndef __SMILE_ENV_SYMBOLTABLE_H__
#include <smile/env/symboltable.h>
#endif

// Preregistered symbol IDs for the special symbols.
typedef enum SmileSpecialSymbolEnum {

	SMILE_SPECIAL_SYMBOL_EQUALS	= 1,
	SMILE_SPECIAL_SYMBOL_OP_EQUALS	= 2,
		
	SMILE_SPECIAL_SYMBOL_IF	= 3,
	SMILE_SPECIAL_SYMBOL_UNLESS	= 4,
	SMILE_SPECIAL_SYMBOL_WHILE	= 5,
	SMILE_SPECIAL_SYMBOL_UNTIL	= 6,
	SMILE_SPECIAL_SYMBOL_TILL	= 7,
		
	SMILE_SPECIAL_SYMBOL_VAR	= 8,
	SMILE_SPECIAL_SYMBOL_CONST	= 9,
	SMILE_SPECIAL_SYMBOL_AUTO	= 10,
		
	SMILE_SPECIAL_SYMBOL_TRY	= 11,
	SMILE_SPECIAL_SYMBOL_CATCH	= 12,
		
	SMILE_SPECIAL_SYMBOL_FN	= 13,
	SMILE_SPECIAL_SYMBOL_QUOTE	= 14,
	SMILE_SPECIAL_SYMBOL_SCOPE	= 15,
	SMILE_SPECIAL_SYMBOL_PROG1	= 16,
	SMILE_SPECIAL_SYMBOL_PROGN	= 17,
	SMILE_SPECIAL_SYMBOL_RETURN	= 18,
		
	SMILE_SPECIAL_SYMBOL_NOT	= 19,
	SMILE_SPECIAL_SYMBOL_OR	= 20,
	SMILE_SPECIAL_SYMBOL_AND	= 21,
		
	SMILE_SPECIAL_SYMBOL_NEW	= 22,
	SMILE_SPECIAL_SYMBOL_IS	= 23,
	SMILE_SPECIAL_SYMBOL_TYPEOF	= 24,
	SMILE_SPECIAL_SYMBOL_SUPEREQ	= 25,
	SMILE_SPECIAL_SYMBOL_SUPERNE	= 26,
		
	SMILE_SPECIAL_SYMBOL_EQ	= 27,
	SMILE_SPECIAL_SYMBOL_NE	= 28,
	SMILE_SPECIAL_SYMBOL_LT	= 29,
	SMILE_SPECIAL_SYMBOL_GT	= 30,
	SMILE_SPECIAL_SYMBOL_LE	= 31,
	SMILE_SPECIAL_SYMBOL_GE	= 32,
		
	SMILE_SPECIAL_SYMBOL_PLUS	= 33,
	SMILE_SPECIAL_SYMBOL_MINUS	= 34,
	SMILE_SPECIAL_SYMBOL_STAR	= 35,
	SMILE_SPECIAL_SYMBOL_SLASH	= 36,
		
	SMILE_SPECIAL_SYMBOL_LEFTPARENTHESIS	= 37,
	SMILE_SPECIAL_SYMBOL_RIGHTPARENTHESIS	= 38,
	SMILE_SPECIAL_SYMBOL_LEFTBRACKET	= 39,
	SMILE_SPECIAL_SYMBOL_RIGHTBRACKET	= 40,
	SMILE_SPECIAL_SYMBOL_LEFTBRACE	= 41,
	SMILE_SPECIAL_SYMBOL_RIGHTBRACE	= 42,
		
	SMILE_SPECIAL_SYMBOL_COMMA	= 43,
	SMILE_SPECIAL_SYMBOL_SEMICOLON	= 44,
	SMILE_SPECIAL_SYMBOL_COLON	= 45,
	SMILE_SPECIAL_SYMBOL_QUESTIONMARK	= 46,
		
	SMILE_SPECIAL_SYMBOL_STMT	= 47,
	SMILE_SPECIAL_SYMBOL_EXPR	= 48,
	SMILE_SPECIAL_SYMBOL_CMP	= 49,
	SMILE_SPECIAL_SYMBOL_ADDSUB	= 50,
	SMILE_SPECIAL_SYMBOL_MULDIV	= 51,
	SMILE_SPECIAL_SYMBOL_BINARY	= 52,
	SMILE_SPECIAL_SYMBOL_UNARY	= 53,
	SMILE_SPECIAL_SYMBOL_POSTFIX	= 54,
	SMILE_SPECIAL_SYMBOL_TERM	= 55,
		
	SMILE_SPECIAL_SYMBOL_BRK	= 56,

} SmileSpecialSymbol;

// The set of known symbols, preregistered at startup time to save on runtime-initialization costs.
typedef struct KnownSymbolsStruct {

	// Specials.
	Symbol equals_, op_equals_;
	Symbol if_, unless_, while_, until_, till_;
	Symbol var_, const_, auto_;
	Symbol try_, catch_;
	Symbol fn_, quote_, scope_, prog1_, progn_, return_;
	Symbol not_, or_, and_;
	Symbol new_, is_, typeof_, supereq_, superne_;
	Symbol brk_;
	Symbol eq, ne, lt, gt, le, ge;
	Symbol plus, minus, star, slash;

	// Special syntax nonterminals.
	Symbol STMT, EXPR, CMP, ADDSUB, MULDIV, BINARY, UNARY, POSTFIX, TERM;

	// Operator symbols.
	Symbol caret;
	Symbol shift_left, shift_right, arithmetic_shift_left, arithmetic_shift_right, rotate_left, rotate_right;

	// Special punctuation.
	Symbol left_parenthesis, right_parenthesis, left_bracket, right_bracket, left_brace, right_brace;
	Symbol comma, semicolon, colon, question_mark;
	Symbol implies;

	// Typename symbols.
	Symbol Actor_, Array_, ArrayBase_, Bool_, Byte_, ByteRange_, ByteArray_, Char_, Closure, Enumerable_, Exception_, Facade_, FacadeProper_, Fn_, Handle_;
	Symbol IntegerArrayBase_, Integer_, Integer16_, Integer32_, Integer32Array_, Integer32Map_, Integer32Range_, Integer64_, Integer64Array_, Integer64Map_, Integer64Range_, IntegerBase_, IntegerRange_, IntegerRangeBase_;
	Symbol List_, Map_, MapBase_, MathException, Number_, NumericArrayBase_, Null_, Object_, Pair_, Program_, Random_, Range_;
	Symbol RealArrayBase_, Real_, Real32_, Real32Array_, Real32Range_, Real64_, Real64Array_, Real64Range_, RealBase_, RealRange_, RealRangeBase_;
	Symbol Regex_, String_, StringMap_, Symbol_, SymbolMap_, UChar_, UserObject_;

	// General symbols.
	Symbol a, abs, acos, add_c_slashes, alnum_q, alpha_q, apply, apply_method, arguments, asin, assertions, assigned_name, atan, atan2;
	Symbol base_, bit_and, bit_not, bit_or, bit_xor, body, bool_, byte_, byte_array;
	Symbol call, call_method, camelCase, CamelCase, case_fold, case_insensitive, case_sensitive, category, ceil, char_, chip, chop;
	Symbol cident_q, clip, clone, code_at, code_length, compare, compare_i, compose, composed_q, cons, contains, contains_i, control_q, context, cos, count, count64;
	Symbol count_left_ones, count_left_zeros, count_of, count_of_i, count_ones, count_right_ones, count_right_zeros, count_zeros, crc32, create, create_child_closure;
	Symbol d, decompose, diacritic_q, digit_q, div, divide_by_zero, does_not_understand;
	Symbol each, end, ends_with, ends_with_i, escape, eval, even_q, exit, exp, extend_object, extend_where_new;
	Symbol false_, filename_mode, first, floor, fold, from_seed;
	Symbol get_member, get_object_security, get_property;
	Symbol handle_kind, has_property, hash, hex_string, hex_string_pretty, html_decode, html_encode, hyphenize;
	Symbol id, in_, include, index_of, index_of_i, int_, int16_, int32_, int64_, int_lg;
	Symbol join;
	Symbol keys, kind;
	Symbol last_index_of, last_index_of_i, latin1_to_utf8, left, length, letter, letter_q;
	Symbol letter_lowercase, letter_modifier, letter_other, letter_titlecase, letter_uppercase;
	Symbol lg, list, load, log, log_domain, lower, lowercase, lowercase_q, ln;
	Symbol map, mark, mark_enclosing, mark_non_spacing, mark_spacing_combining;
	Symbol match, matches, max, message, mid, min, mod;
	Symbol name, neg_q, newline_q, next_pow2, nonterminal, normalize_diacritics, nth, nth_cell, null_, number;
	Symbol number_decimal_digit, number_letter, number_other, numeric_q;
	Symbol octal_q, of, of_size, odd_q, one_q, options, other;
	Symbol other_control, other_format, other_not_assigned, other_private_use, other_surrogate;
	Symbol parity, parse, parse_and_eval, pattern, pos_q, post, pow2_q, pre, primary_category, printf, process_id, property_names, punct_q, punctuation;
	Symbol punctuation_close, punctuation_connector, punctuation_dash, punctuation_final_quote, punctuation_initial_quote, punctuation_open, punctuation_other;
	Symbol raw_reverse, read_append, read_only, read_write, read_write_append, real_, real32_, real64_, rem, repeat, replace, replacement, resize, rest, result;
	Symbol reverse, reverse_bits, reverse_bytes, right, rot_13;
	Symbol separator, separator_line, separator_paragraph, separator_space;
	Symbol set_member, set_object_security, set_property, sign, sin, space_q, splice, split, sprintf;
	Symbol sqrt, sqrt_domain, start, starts_with, starts_with_i, step, stepping;
	Symbol string_, strip_c_slashes, studied_, study, substr, substring, symbol;
	Symbol symbol_currency, symbol_math, symbol_modifier, symbol_other;
	Symbol tan, text, this_, this_closure, throw_, title, titlecase, titlecase_q, trim, trim_end, trim_start, true_, type;
	Symbol uchar, underscorize, unknown, upper, uppercase, uppercase_q, url_decode, url_encode, url_query_encode, utf8_to_latin1;
	Symbol values;
	Symbol where_, whitespace_q, wildcard_matches, without;
	Symbol xdigit_q, xor;
	Symbol zero_q;

	// Error and exception symbols.
	Symbol compile_error, configuration_error, eval_error, exec_error, json_error, lexer_error, load_error, native_method_error, object_security_error;
	Symbol post_condition_assertion, pre_condition_assertion, property_error, syntax_error, system_exception, type_assertion, user_exception;

} *KnownSymbols;

SMILE_API_FUNC void KnownSymbols_PreloadSymbolTable(SymbolTable symbolTable, KnownSymbols knownSymbols);

#endif