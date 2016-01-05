//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2016 Sean Werkema
//
//  Licensed under the Apache Licens  E, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//---------------------------------------------------------------------------------------

#include <smile/types.h>
#include <smile/parsing/identkind.h>

#define _	(0)
#define A	(IDENTKIND_STARTLETTER)
#define M	(IDENTKIND_MIDDLELETTER)
#define P	(IDENTKIND_PUNCTUATION)
#define E	(IDENTKIND_PUNCTUATION | IDENTKIND_MIDDLELETTER)

#define Any	(0)
#define L	(IDENTKIND_CHARSET_LATIN)
#define Gr	(IDENTKIND_CHARSET_GREEK)
#define Cy	(IDENTKIND_CHARSET_CYRILLIC)
#define Ar	(IDENTKIND_CHARSET_ARMENIAN)
#define He	(IDENTKIND_CHARSET_HEBREW)

static const UInt16 _disallowed[] = {
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 00 - 0F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 10 - 1F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 20 - 2F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 30 - 3F

	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 40 - 4F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 50 - 5F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 60 - 6F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 70 - 7F

	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 80 - 8F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 90 - 9F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// A0 - AF
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// B0 - BF

	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// C0 - CF
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// D0 - DF
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// E0 - EF
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// F0 - FF
};

static const UInt16 _identTable00[] = {
	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// 00 - 0F
	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// 10 - 1F
	  _,   E,   M,   _,   A,   P,   P,   A,   _,   _,   P,   P,   _,   E,   _,   P,		// 20 - 2F
	  M,   M,   M,   M,   M,   M,   M,   M,   M,   M,   _,   _,   P,   P,   P,   E,		// 30 - 3F

	  P, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 40 - 4F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,   _,   _,   _,   P, L|A,		// 50 - 5F
	  _, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 60 - 6F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,   _,   _,   _,   P,   _,		// 70 - 7F

	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// 80 - 8F
	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// 90 - 9F
	  _,   P,   A,   A,   A,   A,   _,   A,   _,   A, L|M,   P,   P,   _,   A,   _,		// A0 - AF
	  M,   P,   M,   M,   _, L|A, L|A,   P,   _,   M, L|M,   P,   _,   _,   _,   P,		// B0 - BF

	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// C0 - CF
	L|A, L|A, L|A, L|A, L|A, L|A, L|A,   _, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// D0 - DF
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// E0 - EF
	L|A, L|A, L|A, L|A, L|A, L|A, L|A,   _, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// F0 - FF
};

static const UInt16 _identTable01[] = {
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 00 - 0F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 10 - 1F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 20 - 2F
	L|A, L|A,   _,   _, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 30 - 3F

	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 40 - 4F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 50 - 5F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 60 - 6F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 70 - 7F

	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 80 - 8F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 90 - 9F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// A0 - AF
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// B0 - BF

	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _, L|A, L|A, L|A,		// C0 - CF
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// D0 - DF
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// E0 - EF
	L|A,   _,   _,   _, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// F0 - FF
};

static const UInt16 _identTable02[] = {
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 00 - 0F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 10 - 1F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 20 - 2F
	L|A, L|A,   _,   _, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 30 - 3F

	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 40 - 4F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 50 - 5F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 60 - 6F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 70 - 7F

	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 80 - 8F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// 90 - 9F
	L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A, L|A,		// A0 - AF
	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// B0 - BF

	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// C0 - CF
	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// D0 - DF
	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// E0 - EF
	  _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,   _,		// F0 - FF
};

static const UInt16 _identTable03[] = {
	  E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,		// 00 - 0F
	  E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,		// 10 - 1F
	  E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,		// 20 - 2F
	  E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,		// 30 - 3F

	  E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,		// 40 - 4F
	  E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,		// 50 - 5F
	  E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,   E,		// 60 - 6F

	Gr|A, Gr|A, Gr|A, Gr|A,    _,    _, Gr|A, Gr|A, Gr|A,    _,    _,    _, Gr|A, Gr|A,    _, Gr|A,		// 70 - 7F

	   _,    _,    _,    _,    _,    _, Gr|A,    _, Gr|A, Gr|A, Gr|A,    _, Gr|A,    _, Gr|A, Gr|A,		// 80 - 8F
	Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A,		// 90 - 9F
	Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A,		// A0 - AF
	Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A,		// B0 - BF

	Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A,		// C0 - CF
	Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A,		// D0 - DF
	Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A,		// E0 - EF
	Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A, Gr|A,		// F0 - FF
};

static const UInt16 _identTable04[] = {
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 00 - 0F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 10 - 1F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 20 - 2F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 30 - 3F

	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 40 - 4F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 50 - 5F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 60 - 6F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 70 - 7F

	Cy | A, Cy | A, Cy | M, Cy | M, Cy | M, Cy | M, Cy | M, Cy | M, Cy | M, Cy | M, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 80 - 8F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 90 - 9F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// A0 - AF
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// B0 - BF

	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// C0 - CF
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// D0 - DF
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// E0 - EF
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// F0 - FF
};

static const UInt16 _identTable05[] = {
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 00 - 0F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 10 - 1F
	Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A, Cy | A,		// 20 - 2F

	_, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A,		// 30 - 3F

	Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A,		// 40 - 4F
	Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, _, _, _, _, _, _, _, _, _,		// 50 - 5F
	Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A,		// 60 - 6F
	Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A,		// 70 - 7F

	Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, Ar | A, _, _, Ar | M, _, _, Ar | A, Ar | A, Ar | A,		// 80 - 8F

	_, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M,		// 90 - 9F
	He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M,		// A0 - AF
	He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, He | M, _, He | M,		// B0 - BF

	_, He | M, He | M, _, He | M, He | M, _, He | M, _, _, _, _, _, _, _, _,		// C0 - CF
	He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A,		// D0 - DF
	He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, He | A, _, _, _, _, _,		// E0 - EF
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,		// F0 - FF
};

static const UInt16 _identTable20[] = {
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 00 - 0F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 10 - 1F
	M, M, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 20 - 2F
	M, M, _, _, _, _, _, _, _, _, _, P, _, P, _, _,			// 30 - 3F

	_, P, P, _, _, _, _, _, _, _, _, A, _, _, _, _,			// 40 - 4F
	P, P, _, _, _, P, _, _, _, _, _, _, P, _, _, _,			// 50 - 5F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 60 - 6F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 70 - 7F

	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 80 - 8F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 90 - 9F
	A, A, A, A, A, A, A, A, A, A, A, A, A, A, A, A,			// A0 - AF
	A, A, A, A, A, A, A, A, A, A, A, A, A, A, _, _,			// B0 - BF

	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// C0 - CF
	E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,			// D0 - DF
	E, E, E, E, E, E, E, E, E, E, E, E, E, E, E, E,			// E0 - EF
	E, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// F0 - FF
};

static const UInt16 _identTable21[] = {
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 00 - 0F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 10 - 1F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 20 - 2F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 30 - 3F

	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 40 - 4F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 50 - 5F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 60 - 6F
	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 70 - 7F

	_, _, _, _, _, _, _, _, _, _, _, _, _, _, _, _,			// 80 - 8F
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// 90 - 9F
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// A0 - AF
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// B0 - BF

	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// C0 - CF
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// D0 - DF
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// E0 - EF
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// F0 - FF
};

static const UInt16 _identTable22[] = {
	P, _, A, P, P, A, P, P, P, P, _, P, P, _, _, P,			// 00 - 0F
	P, P, _, P, P, _, _, _, P, _, P, P, P, P, A, A,			// 10 - 1F
	A, A, A, _, _, _, _, P, P, P, P, P, _, _, P, P,			// 20 - 2F
	P, P, P, P, P, P, _, _, P, _, P, P, P, P, P, P,			// 30 - 3F

	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// 40 - 4F
	P, P, P, P, _, _, P, P, P, P, P, P, P, P, P, P,			// 50 - 5F
	P, P, P, P, P, P, P, P, P, P, _, _, P, P, P, P,			// 60 - 6F
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// 70 - 7F

	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// 80 - 8F
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// 90 - 9F
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// A0 - AF
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// B0 - BF

	P, P, P, P, P, _, _, P, P, P, P, P, P, P, P, P,			// C0 - CF
	P, P, P, P, P, _, P, P, _, _, P, P, P, P, P, P,			// D0 - DF
	P, P, P, P, P, P, P, P, P, P, P, P, P, P, _, _,			// E0 - EF
	_, _, P, P, P, P, P, P, P, P, P, P, P, P, P, P,			// F0 - FF
};

const UInt16 *SmileIdentifierTable[] = {
	_identTable00, _identTable01, _identTable02, _identTable03, _identTable04, _identTable05, _disallowed, _disallowed,
	_disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed,

	_disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed,
	_disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed, _disallowed,

	_identTable20, _identTable21, _identTable22,
};

const UInt32 SmileIdentifierTableLength = sizeof(SmileIdentifierTable) / sizeof(const UInt16 *);
