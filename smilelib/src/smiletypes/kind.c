//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2017 Sean Werkema
//
//  Licensed under the Apache License, Version 2.0 (the "License");
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

#include <smile/smiletypes/smileobject.h>
#include <smile/string.h>
#include <smile/internal/staticstring.h>

STATIC_STRING(UnboxedByte, "Byte");
STATIC_STRING(UnboxedInteger16, "Integer16");
STATIC_STRING(UnboxedInteger32, "Integer32");
STATIC_STRING(UnboxedInteger64, "Integer64");
STATIC_STRING(UnboxedBool, "Bool");
STATIC_STRING(UnboxedChar, "Char");
STATIC_STRING(UnboxedUni, "Uni");
STATIC_STRING(UnboxedFloat32, "Float32");
STATIC_STRING(UnboxedFloat64, "Float64");
STATIC_STRING(UnboxedSymbol, "Symbol");
STATIC_STRING(UnboxedReal32, "Real32");
STATIC_STRING(UnboxedReal64, "Real64");

STATIC_STRING(BoxedByte, "boxed Byte");
STATIC_STRING(BoxedInteger16, "boxed Integer16");
STATIC_STRING(BoxedInteger32, "boxed Integer32");
STATIC_STRING(BoxedInteger64, "boxed Integer64");
STATIC_STRING(BoxedBool, "boxed Bool");
STATIC_STRING(BoxedChar, "boxed Char");
STATIC_STRING(BoxedUni, "boxed Uni");
STATIC_STRING(BoxedFloat32, "boxed Float32");
STATIC_STRING(BoxedFloat64, "boxed Float64");
STATIC_STRING(BoxedSymbol, "boxed Symbol");
STATIC_STRING(BoxedReal32, "boxed Real32");
STATIC_STRING(BoxedReal64, "boxed Real64");

STATIC_STRING(Null_, "Null");
STATIC_STRING(List_, "List");
STATIC_STRING(Primitive_, "Primitive");
STATIC_STRING(Pair_, "Pair");
STATIC_STRING(UserObject_, "Object");
STATIC_STRING(String_, "String");

STATIC_STRING(Handle_, "Handle");
STATIC_STRING(Closure_, "Closure");
STATIC_STRING(Facade_, "Facade");
STATIC_STRING(Macro_, "Macro");
STATIC_STRING(Function_, "Fn");

STATIC_STRING(Integer128_, "Integer128");
STATIC_STRING(BigInt_, "BigInt");
STATIC_STRING(Float128_, "Float128");
STATIC_STRING(BigFloat_, "BigFloat");
STATIC_STRING(Real128_, "Real128");
STATIC_STRING(BigReal_, "bigReal");

STATIC_STRING(ByteArray_, "ByteArray");

STATIC_STRING(Syntax_, "Syntax");
STATIC_STRING(Nonterminal_, "Nonterminal");

STATIC_STRING(ParseDecl_, "ParseDecl");
STATIC_STRING(ParseMessage_, "ParseMessage");

String SmileKind_GetName(Int kind)
{
	switch (kind)
	{
		case SMILE_KIND_UNBOXED_BYTE: return UnboxedByte;
		case SMILE_KIND_UNBOXED_INTEGER16: return UnboxedInteger16;
		case SMILE_KIND_UNBOXED_INTEGER32: return UnboxedInteger32;
		case SMILE_KIND_UNBOXED_INTEGER64: return UnboxedInteger64;
		case SMILE_KIND_UNBOXED_BOOL: return UnboxedBool;
		case SMILE_KIND_UNBOXED_CHAR: return UnboxedChar;
		case SMILE_KIND_UNBOXED_UNI: return UnboxedUni;
		case SMILE_KIND_UNBOXED_FLOAT32: return UnboxedFloat32;
		case SMILE_KIND_UNBOXED_FLOAT64: return UnboxedFloat64;
		case SMILE_KIND_UNBOXED_SYMBOL: return UnboxedSymbol;
		case SMILE_KIND_UNBOXED_REAL32: return UnboxedReal32;
		case SMILE_KIND_UNBOXED_REAL64: return UnboxedReal64;

		case SMILE_KIND_BYTE: return BoxedByte;
		case SMILE_KIND_INTEGER16: return BoxedInteger16;
		case SMILE_KIND_INTEGER32: return BoxedInteger32;
		case SMILE_KIND_INTEGER64: return BoxedInteger64;
		case SMILE_KIND_BOOL: return BoxedBool;
		case SMILE_KIND_CHAR: return BoxedChar;
		case SMILE_KIND_UNI: return BoxedUni;
		case SMILE_KIND_FLOAT32: return BoxedFloat32;
		case SMILE_KIND_FLOAT64: return BoxedFloat64;
		case SMILE_KIND_SYMBOL: return BoxedSymbol;
		case SMILE_KIND_REAL32: return BoxedReal32;
		case SMILE_KIND_REAL64: return BoxedReal64;

		case SMILE_KIND_NULL: return Null_;
		case SMILE_KIND_LIST: return List_;
		case SMILE_KIND_PRIMITIVE: return Primitive_;
		case SMILE_KIND_PAIR: return Pair_;
		case SMILE_KIND_USEROBJECT: return UserObject_;
		case SMILE_KIND_STRING: return String_;

		case SMILE_KIND_HANDLE: return Handle_;
		case SMILE_KIND_CLOSURE: return Closure_;
		case SMILE_KIND_FACADE: return Facade_;
		case SMILE_KIND_MACRO: return Macro_;
		case SMILE_KIND_FUNCTION: return Function_;

		// Bigger numeric types.	
		case SMILE_KIND_INTEGER128: return Integer128_;
		case SMILE_KIND_BIGINT: return BigInt_;
		case SMILE_KIND_FLOAT128: return Float128_;
		case SMILE_KIND_BIGFLOAT: return BigFloat_;
		case SMILE_KIND_REAL128: return Real128_;
		case SMILE_KIND_BIGREAL: return BigReal_;

		// Raw buffer types.
		case SMILE_KIND_BYTEARRAY: return ByteArray_;

		// Types used for parsing.	
		case SMILE_KIND_SYNTAX: return Syntax_;
		case SMILE_KIND_NONTERMINAL: return Nonterminal_;

		// Internal types used during parsing.	
		case SMILE_KIND_PARSEDECL: return ParseDecl_;
		case SMILE_KIND_PARSEMESSAGE: return ParseMessage_;

		default: return String_Empty;
	}
}
