//---------------------------------------------------------------------------------------
//  Smile Programming Language Interpreter
//  Copyright 2004-2015 Sean Werkema
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

#include <smile/internal/html.h>

const HtmlEntity HtmlEntityTable[] = {
	{ { "quot", 4 }, 34 },     { { "amp", 3 }, 38 },      { { "apos", 4 }, 39 },
	{ { "lt", 2 }, 60 },       { { "gt", 2 }, 62 },
			
	{ { "nbsp", 4 }, 160 },    { { "iexcl", 5 }, 161 },   { { "cent", 4 }, 162 },    { { "pound", 5 }, 163 },
	{ { "curren", 6 }, 164 },  { { "yen", 3 }, 165 },     { { "brvbar", 6 }, 166 },  { { "sect", 4 }, 167 },
	{ { "uml", 3 }, 168 },     { { "copy", 4 }, 169 },    { { "ordf", 4 }, 170 },    { { "laquo", 5 }, 171 },
	{ { "not", 3 }, 172 },     { { "shy", 3 }, 173 },     { { "reg", 3 }, 174 },     { { "macr", 4 }, 175 },
	{ { "deg", 3 }, 176 },     { { "plusmn", 6 }, 177 },  { { "sup2", 4 }, 178 },    { { "sup3", 4 }, 179 },
	{ { "acute", 5 }, 180 },   { { "micro", 5 }, 181 },   { { "para", 4 }, 182 },    { { "middot", 6 }, 183 },
	{ { "cedil", 5 }, 184 },   { { "sup1", 4 }, 185 },    { { "ordm", 4 }, 186 },    { { "raquo", 5 }, 187 },
	{ { "frac14", 6 }, 188 },  { { "frac12", 6 }, 189 },  { { "frac34", 6 }, 190 },  { { "iquest", 6 }, 191 },

	{ { "Agrave", 6 }, 192 },  { { "Aacute", 6 }, 193 },  { { "Acirc", 5 }, 194 },   { { "Atilde", 6 }, 195 },
	{ { "Auml", 4 }, 196 },    { { "Aring", 5 }, 197 },   { { "AElig", 5 }, 198 },   { { "Ccedil", 6 }, 199 },
	{ { "Egrave", 6 }, 200 },  { { "Eacute", 6 }, 201 },  { { "Ecirc", 5 }, 202 },   { { "Euml", 4 }, 203 },
	{ { "Igrave", 6 }, 204 },  { { "Iacute", 6 }, 205 },  { { "Icirc", 5 }, 206 },   { { "Iuml", 4 }, 207 },
	{ { "ETH", 3 }, 208 },     { { "Ntilde", 6 }, 209 },  { { "Ograve", 6 }, 210 },  { { "Oacute", 6 }, 211 },
	{ { "Ocirc", 5 }, 212 },   { { "Otilde", 6 }, 213 },  { { "Ouml", 4 }, 214 },    { { "times", 5 }, 215 },
	{ { "Oslash", 6 }, 216 },  { { "Ugrave", 6 }, 217 },  { { "Uacute", 5 }, 218 },  { { "Ucirc", 5 }, 219 },
	{ { "Uuml", 4 }, 220 },    { { "Yacute", 6 }, 221 },  { { "THORN", 5 }, 222 },   { { "szlig", 5 }, 223 },

	{ { "agrave", 6 }, 224 },  { { "aacute", 6 }, 225 },  { { "acirc", 5 }, 226 },   { { "atilde", 6 }, 227 },
	{ { "auml", 4 }, 228 },    { { "aring", 5 }, 229 },   { { "aelig", 5 }, 230 },   { { "ccedil", 6 }, 231 },
	{ { "egrave", 6 }, 232 },  { { "eacute", 6 }, 233 },  { { "ecirc", 5 }, 234 },   { { "euml", 4 }, 235 },
	{ { "igrave", 6 }, 236 },  { { "iacute", 6 }, 237 },  { { "icirc", 5 }, 238 },   { { "iuml", 4 }, 239 },
	{ { "eth", 3 }, 240 },     { { "ntilde", 6 }, 241 },  { { "ograve", 6 }, 242 },  { { "oacute", 6 }, 243 },
	{ { "ocirc", 5 }, 244 },   { { "otilde", 6 }, 245 },  { { "ouml", 4 }, 246 },    { { "divide", 6 }, 247 },
	{ { "oslash", 6 }, 248 },  { { "ugrave", 6 }, 249 },  { { "uacute", 6 }, 250 },  { { "ucirc", 5 }, 251 },
	{ { "uuml", 4 }, 252 },    { { "yacute", 6 }, 253 },  { { "thorn", 5 }, 254 },   { { "yuml", 4 }, 255 },

	{ { "OElig", 5 }, 338 },   { { "oelig", 5 }, 339 },   { { "Scaron", 6 }, 352 },  { { "scaron", 6 }, 353 },
	{ { "Yuml", 4 }, 376 },    { { "fnof", 4 }, 402 },    { { "circ", 4 }, 710 },    { { "tilde", 5 }, 732 },

	{ { "Alpha", 5 }, 913 },   { { "Beta", 4 }, 914 },    { { "Gamma", 5 }, 915 },   { { "Delta", 5 }, 916 },
	{ { "Epsilon", 7 }, 917 }, { { "Zeta", 4 }, 918 },    { { "Eta", 3 }, 919 },     { { "Theta", 5 }, 920 },
	{ { "Iota", 4 }, 921 },    { { "Kappa", 5 }, 922 },   { { "Lambda", 6 }, 923 },  { { "Mu", 2 }, 924 },
	{ { "Nu", 2 }, 925 },      { { "Xi", 2 }, 926 },      { { "Omicron", 7 }, 927 }, { { "Pi", 2 }, 928 },
	{ { "Rho", 3 }, 929 },     { { "Sigma", 5 }, 931 },   { { "Tau", 3 }, 932 },     { { "Upsilon", 7 }, 933 },
	{ { "Phi", 3 }, 934 },     { { "Chi", 3 }, 935 },     { { "Psi", 3 }, 936 },     { { "Omega", 5 }, 937 },
			
	{ { "alpha", 5 }, 945 },   { { "beta", 4 }, 946 },    { { "gamma", 5 }, 947 },   { { "delta", 5 }, 948 },
	{ { "epsilon", 7 }, 949 }, { { "zeta", 4 }, 950 },    { { "eta", 3 }, 951 },     { { "theta", 5 }, 952 },
	{ { "iota", 4 }, 953 },    { { "kappa", 5 }, 954 },   { { "lambda", 6 }, 955 },  { { "mu", 2 }, 956 },
	{ { "nu", 2 }, 957 },      { { "xi", 2 }, 958 },      { { "omicron", 7 }, 959 }, { { "pi", 2 }, 960 },
	{ { "rho", 3 }, 961 },     { { "sigmaf", 6 }, 962 },  { { "sigma", 5 }, 963 },   { { "tau", 3 }, 964 },
	{ { "upsilon", 7 }, 965 }, { { "phi", 3 }, 966 },     { { "chi", 3 }, 967 },     { { "psi", 3 }, 968 },
	{ { "omega", 5 }, 969 },   { { "thetasym", 8 }, 977 },{ { "upsih", 5 }, 978 },   { { "piv", 3 }, 982 },

	{ { "ensp", 4 }, 8194 },   { { "emsp", 4 }, 8195 },   { { "thinsp", 6 }, 8201 }, { { "zwnj", 4 }, 8204 },
	{ { "zwj", 3 }, 8205 },    { { "lrm", 3 }, 8206 },    { { "rlm", 3 }, 8207 },    { { "ndash", 5 }, 8211 },
	{ { "mdash", 5 }, 8212 },  { { "lsquo", 5 }, 8216 },  { { "rsquo", 5 }, 8217 },  { { "sbquo", 5 }, 8218 },
	{ { "ldquo", 5 }, 8220 },  { { "rdquo", 5 }, 8221 },  { { "bdquo", 5 }, 8222 },  { { "dagger", 6 }, 8224 },
	{ { "Dagger", 6 }, 8225 }, { { "bull", 4 }, 8226 },   { { "hellip", 6 }, 8230 }, { { "permil", 6 }, 8240 },
	{ { "prime", 5 }, 8242 },  { { "Prime", 5 }, 8243 },  { { "lsaquo", 6 }, 8249 }, { { "rsaquo", 6 }, 8250 },
	{ { "oline", 5 }, 8254 },  { { "frasl", 5 }, 8260 },  { { "euro", 4 }, 8364 },   { { "image", 5 }, 8465 },
	{ { "weierp", 6 }, 8472 }, { { "real", 4 }, 8476 },   { { "trade", 5 }, 8482 },  { { "alefsym", 7 }, 8501 },
	{ { "larr", 4 }, 8592 },   { { "uarr", 4 }, 8593 },   { { "rarr", 4 }, 8594 },   { { "darr", 4 }, 8595 },
	{ { "harr", 4 }, 8596 },   { { "crarr", 5 }, 8629 },  { { "lArr", 4 }, 8656 },   { { "uArr", 4 }, 8657 },
	{ { "rArr", 4 }, 8658 },   { { "dArr", 4 }, 8659 },   { { "hArr", 4 }, 8660 },   { { "forall", 6 }, 8704 },
	{ { "part", 4 }, 8706 },   { { "exist", 5 }, 8707 },  { { "empty", 5 }, 8709 },  { { "nabla", 5 }, 8711 },
	{ { "isin", 4 }, 8712 },   { { "notin", 5 }, 8713 },  { { "ni", 2 }, 8715 },     { { "prod", 4 }, 8719 },
	{ { "sum", 3 }, 8721 },    { { "minus", 5 }, 8722 },  { { "lowast", 6 }, 8727 }, { { "radic", 5 }, 8730 },
	{ { "prop", 4 }, 8733 },   { { "infin", 5 }, 8734 },  { { "ang", 3 }, 8736 },    { { "and", 3 }, 8743 },
	{ { "or", 2 }, 8744 },     { { "cap", 3 }, 8745 },    { { "cup", 3 }, 8746 },    { { "int", 3 }, 8747 },
	{ { "there4", 6 }, 8756 }, { { "sim", 3 }, 8764 },    { { "cong", 4 }, 8773 },   { { "asymp", 5 }, 8776 },
	{ { "ne", 2 }, 8800 },     { { "equiv", 5 }, 8801 },  { { "le", 2 }, 8804 },     { { "ge", 2 }, 8805 },
	{ { "sub", 3 }, 8834 },    { { "sup", 3 }, 8835 },    { { "nsub", 4 }, 8836 },   { { "sube", 4 }, 8838 },
	{ { "supe", 4 }, 8839 },   { { "oplus", 5 }, 8853 },  { { "otimes", 6 }, 8855 }, { { "perp", 4 }, 8869 },
	{ { "sdot", 4 }, 8901 },   { { "vellip", 6 }, 8942 }, { { "lceil", 5 }, 8968 },  { { "rceil", 5 }, 8969 },
	{ { "lfloor", 6 }, 8970 }, { { "rfloor", 6 }, 8971 }, { { "lang", 4 }, 9001 },   { { "rang", 4 }, 9002 },
	{ { "loz", 3 }, 9674 },    { { "spades", 6 }, 9824 }, { { "clubs", 4 }, 9827 },  { { "hearts", 6 }, 9829 },
	{ { "diams", 5 }, 9830 },
};

const Int HtmlEntityTableLength = sizeof(HtmlEntityTable) / sizeof(HtmlEntity);
