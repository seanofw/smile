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

#include <smile/internal/html.h>

const HtmlEntity HtmlEntityTable[] = {
	{ { (Byte *)"quot", 4 }, 34 },     { { (Byte *)"amp", 3 }, 38 },      { { (Byte *)"apos", 4 }, 39 },
	{ { (Byte *)"lt", 2 }, 60 },       { { (Byte *)"gt", 2 }, 62 },
			
	{ { (Byte *)"nbsp", 4 }, 160 },    { { (Byte *)"iexcl", 5 }, 161 },   { { (Byte *)"cent", 4 }, 162 },    { { (Byte *)"pound", 5 }, 163 },
	{ { (Byte *)"curren", 6 }, 164 },  { { (Byte *)"yen", 3 }, 165 },     { { (Byte *)"brvbar", 6 }, 166 },  { { (Byte *)"sect", 4 }, 167 },
	{ { (Byte *)"uml", 3 }, 168 },     { { (Byte *)"copy", 4 }, 169 },    { { (Byte *)"ordf", 4 }, 170 },    { { (Byte *)"laquo", 5 }, 171 },
	{ { (Byte *)"not", 3 }, 172 },     { { (Byte *)"shy", 3 }, 173 },     { { (Byte *)"reg", 3 }, 174 },     { { (Byte *)"macr", 4 }, 175 },
	{ { (Byte *)"deg", 3 }, 176 },     { { (Byte *)"plusmn", 6 }, 177 },  { { (Byte *)"sup2", 4 }, 178 },    { { (Byte *)"sup3", 4 }, 179 },
	{ { (Byte *)"acute", 5 }, 180 },   { { (Byte *)"micro", 5 }, 181 },   { { (Byte *)"para", 4 }, 182 },    { { (Byte *)"middot", 6 }, 183 },
	{ { (Byte *)"cedil", 5 }, 184 },   { { (Byte *)"sup1", 4 }, 185 },    { { (Byte *)"ordm", 4 }, 186 },    { { (Byte *)"raquo", 5 }, 187 },
	{ { (Byte *)"frac14", 6 }, 188 },  { { (Byte *)"frac12", 6 }, 189 },  { { (Byte *)"frac34", 6 }, 190 },  { { (Byte *)"iquest", 6 }, 191 },

	{ { (Byte *)"Agrave", 6 }, 192 },  { { (Byte *)"Aacute", 6 }, 193 },  { { (Byte *)"Acirc", 5 }, 194 },   { { (Byte *)"Atilde", 6 }, 195 },
	{ { (Byte *)"Auml", 4 }, 196 },    { { (Byte *)"Aring", 5 }, 197 },   { { (Byte *)"AElig", 5 }, 198 },   { { (Byte *)"Ccedil", 6 }, 199 },
	{ { (Byte *)"Egrave", 6 }, 200 },  { { (Byte *)"Eacute", 6 }, 201 },  { { (Byte *)"Ecirc", 5 }, 202 },   { { (Byte *)"Euml", 4 }, 203 },
	{ { (Byte *)"Igrave", 6 }, 204 },  { { (Byte *)"Iacute", 6 }, 205 },  { { (Byte *)"Icirc", 5 }, 206 },   { { (Byte *)"Iuml", 4 }, 207 },
	{ { (Byte *)"ETH", 3 }, 208 },     { { (Byte *)"Ntilde", 6 }, 209 },  { { (Byte *)"Ograve", 6 }, 210 },  { { (Byte *)"Oacute", 6 }, 211 },
	{ { (Byte *)"Ocirc", 5 }, 212 },   { { (Byte *)"Otilde", 6 }, 213 },  { { (Byte *)"Ouml", 4 }, 214 },    { { (Byte *)"times", 5 }, 215 },
	{ { (Byte *)"Oslash", 6 }, 216 },  { { (Byte *)"Ugrave", 6 }, 217 },  { { (Byte *)"Uacute", 5 }, 218 },  { { (Byte *)"Ucirc", 5 }, 219 },
	{ { (Byte *)"Uuml", 4 }, 220 },    { { (Byte *)"Yacute", 6 }, 221 },  { { (Byte *)"THORN", 5 }, 222 },   { { (Byte *)"szlig", 5 }, 223 },

	{ { (Byte *)"agrave", 6 }, 224 },  { { (Byte *)"aacute", 6 }, 225 },  { { (Byte *)"acirc", 5 }, 226 },   { { (Byte *)"atilde", 6 }, 227 },
	{ { (Byte *)"auml", 4 }, 228 },    { { (Byte *)"aring", 5 }, 229 },   { { (Byte *)"aelig", 5 }, 230 },   { { (Byte *)"ccedil", 6 }, 231 },
	{ { (Byte *)"egrave", 6 }, 232 },  { { (Byte *)"eacute", 6 }, 233 },  { { (Byte *)"ecirc", 5 }, 234 },   { { (Byte *)"euml", 4 }, 235 },
	{ { (Byte *)"igrave", 6 }, 236 },  { { (Byte *)"iacute", 6 }, 237 },  { { (Byte *)"icirc", 5 }, 238 },   { { (Byte *)"iuml", 4 }, 239 },
	{ { (Byte *)"eth", 3 }, 240 },     { { (Byte *)"ntilde", 6 }, 241 },  { { (Byte *)"ograve", 6 }, 242 },  { { (Byte *)"oacute", 6 }, 243 },
	{ { (Byte *)"ocirc", 5 }, 244 },   { { (Byte *)"otilde", 6 }, 245 },  { { (Byte *)"ouml", 4 }, 246 },    { { (Byte *)"divide", 6 }, 247 },
	{ { (Byte *)"oslash", 6 }, 248 },  { { (Byte *)"ugrave", 6 }, 249 },  { { (Byte *)"uacute", 6 }, 250 },  { { (Byte *)"ucirc", 5 }, 251 },
	{ { (Byte *)"uuml", 4 }, 252 },    { { (Byte *)"yacute", 6 }, 253 },  { { (Byte *)"thorn", 5 }, 254 },   { { (Byte *)"yuml", 4 }, 255 },

	{ { (Byte *)"OElig", 5 }, 338 },   { { (Byte *)"oelig", 5 }, 339 },   { { (Byte *)"Scaron", 6 }, 352 },  { { (Byte *)"scaron", 6 }, 353 },
	{ { (Byte *)"Yuml", 4 }, 376 },    { { (Byte *)"fnof", 4 }, 402 },    { { (Byte *)"circ", 4 }, 710 },    { { (Byte *)"tilde", 5 }, 732 },

	{ { (Byte *)"Alpha", 5 }, 913 },   { { (Byte *)"Beta", 4 }, 914 },    { { (Byte *)"Gamma", 5 }, 915 },   { { (Byte *)"Delta", 5 }, 916 },
	{ { (Byte *)"Epsilon", 7 }, 917 }, { { (Byte *)"Zeta", 4 }, 918 },    { { (Byte *)"Eta", 3 }, 919 },     { { (Byte *)"Theta", 5 }, 920 },
	{ { (Byte *)"Iota", 4 }, 921 },    { { (Byte *)"Kappa", 5 }, 922 },   { { (Byte *)"Lambda", 6 }, 923 },  { { (Byte *)"Mu", 2 }, 924 },
	{ { (Byte *)"Nu", 2 }, 925 },      { { (Byte *)"Xi", 2 }, 926 },      { { (Byte *)"Omicron", 7 }, 927 }, { { (Byte *)"Pi", 2 }, 928 },
	{ { (Byte *)"Rho", 3 }, 929 },     { { (Byte *)"Sigma", 5 }, 931 },   { { (Byte *)"Tau", 3 }, 932 },     { { (Byte *)"Upsilon", 7 }, 933 },
	{ { (Byte *)"Phi", 3 }, 934 },     { { (Byte *)"Chi", 3 }, 935 },     { { (Byte *)"Psi", 3 }, 936 },     { { (Byte *)"Omega", 5 }, 937 },
			
	{ { (Byte *)"alpha", 5 }, 945 },   { { (Byte *)"beta", 4 }, 946 },    { { (Byte *)"gamma", 5 }, 947 },   { { (Byte *)"delta", 5 }, 948 },
	{ { (Byte *)"epsilon", 7 }, 949 }, { { (Byte *)"zeta", 4 }, 950 },    { { (Byte *)"eta", 3 }, 951 },     { { (Byte *)"theta", 5 }, 952 },
	{ { (Byte *)"iota", 4 }, 953 },    { { (Byte *)"kappa", 5 }, 954 },   { { (Byte *)"lambda", 6 }, 955 },  { { (Byte *)"mu", 2 }, 956 },
	{ { (Byte *)"nu", 2 }, 957 },      { { (Byte *)"xi", 2 }, 958 },      { { (Byte *)"omicron", 7 }, 959 }, { { (Byte *)"pi", 2 }, 960 },
	{ { (Byte *)"rho", 3 }, 961 },     { { (Byte *)"sigmaf", 6 }, 962 },  { { (Byte *)"sigma", 5 }, 963 },   { { (Byte *)"tau", 3 }, 964 },
	{ { (Byte *)"upsilon", 7 }, 965 }, { { (Byte *)"phi", 3 }, 966 },     { { (Byte *)"chi", 3 }, 967 },     { { (Byte *)"psi", 3 }, 968 },
	{ { (Byte *)"omega", 5 }, 969 },   { { (Byte *)"thetasym", 8 }, 977 },{ { (Byte *)"upsih", 5 }, 978 },   { { (Byte *)"piv", 3 }, 982 },

	{ { (Byte *)"ensp", 4 }, 8194 },   { { (Byte *)"emsp", 4 }, 8195 },   { { (Byte *)"thinsp", 6 }, 8201 }, { { (Byte *)"zwnj", 4 }, 8204 },
	{ { (Byte *)"zwj", 3 }, 8205 },    { { (Byte *)"lrm", 3 }, 8206 },    { { (Byte *)"rlm", 3 }, 8207 },    { { (Byte *)"ndash", 5 }, 8211 },
	{ { (Byte *)"mdash", 5 }, 8212 },  { { (Byte *)"lsquo", 5 }, 8216 },  { { (Byte *)"rsquo", 5 }, 8217 },  { { (Byte *)"sbquo", 5 }, 8218 },
	{ { (Byte *)"ldquo", 5 }, 8220 },  { { (Byte *)"rdquo", 5 }, 8221 },  { { (Byte *)"bdquo", 5 }, 8222 },  { { (Byte *)"dagger", 6 }, 8224 },
	{ { (Byte *)"Dagger", 6 }, 8225 }, { { (Byte *)"bull", 4 }, 8226 },   { { (Byte *)"hellip", 6 }, 8230 }, { { (Byte *)"permil", 6 }, 8240 },
	{ { (Byte *)"prime", 5 }, 8242 },  { { (Byte *)"Prime", 5 }, 8243 },  { { (Byte *)"lsaquo", 6 }, 8249 }, { { (Byte *)"rsaquo", 6 }, 8250 },
	{ { (Byte *)"oline", 5 }, 8254 },  { { (Byte *)"frasl", 5 }, 8260 },  { { (Byte *)"euro", 4 }, 8364 },   { { (Byte *)"image", 5 }, 8465 },
	{ { (Byte *)"weierp", 6 }, 8472 }, { { (Byte *)"real", 4 }, 8476 },   { { (Byte *)"trade", 5 }, 8482 },  { { (Byte *)"alefsym", 7 }, 8501 },
	{ { (Byte *)"larr", 4 }, 8592 },   { { (Byte *)"uarr", 4 }, 8593 },   { { (Byte *)"rarr", 4 }, 8594 },   { { (Byte *)"darr", 4 }, 8595 },
	{ { (Byte *)"harr", 4 }, 8596 },   { { (Byte *)"crarr", 5 }, 8629 },  { { (Byte *)"lArr", 4 }, 8656 },   { { (Byte *)"uArr", 4 }, 8657 },
	{ { (Byte *)"rArr", 4 }, 8658 },   { { (Byte *)"dArr", 4 }, 8659 },   { { (Byte *)"hArr", 4 }, 8660 },   { { (Byte *)"forall", 6 }, 8704 },
	{ { (Byte *)"part", 4 }, 8706 },   { { (Byte *)"exist", 5 }, 8707 },  { { (Byte *)"empty", 5 }, 8709 },  { { (Byte *)"nabla", 5 }, 8711 },
	{ { (Byte *)"isin", 4 }, 8712 },   { { (Byte *)"notin", 5 }, 8713 },  { { (Byte *)"ni", 2 }, 8715 },     { { (Byte *)"prod", 4 }, 8719 },
	{ { (Byte *)"sum", 3 }, 8721 },    { { (Byte *)"minus", 5 }, 8722 },  { { (Byte *)"lowast", 6 }, 8727 }, { { (Byte *)"radic", 5 }, 8730 },
	{ { (Byte *)"prop", 4 }, 8733 },   { { (Byte *)"infin", 5 }, 8734 },  { { (Byte *)"ang", 3 }, 8736 },    { { (Byte *)"and", 3 }, 8743 },
	{ { (Byte *)"or", 2 }, 8744 },     { { (Byte *)"cap", 3 }, 8745 },    { { (Byte *)"cup", 3 }, 8746 },    { { (Byte *)"int", 3 }, 8747 },
	{ { (Byte *)"there4", 6 }, 8756 }, { { (Byte *)"sim", 3 }, 8764 },    { { (Byte *)"cong", 4 }, 8773 },   { { (Byte *)"asymp", 5 }, 8776 },
	{ { (Byte *)"ne", 2 }, 8800 },     { { (Byte *)"equiv", 5 }, 8801 },  { { (Byte *)"le", 2 }, 8804 },     { { (Byte *)"ge", 2 }, 8805 },
	{ { (Byte *)"sub", 3 }, 8834 },    { { (Byte *)"sup", 3 }, 8835 },    { { (Byte *)"nsub", 4 }, 8836 },   { { (Byte *)"sube", 4 }, 8838 },
	{ { (Byte *)"supe", 4 }, 8839 },   { { (Byte *)"oplus", 5 }, 8853 },  { { (Byte *)"otimes", 6 }, 8855 }, { { (Byte *)"perp", 4 }, 8869 },
	{ { (Byte *)"sdot", 4 }, 8901 },   { { (Byte *)"vellip", 6 }, 8942 }, { { (Byte *)"lceil", 5 }, 8968 },  { { (Byte *)"rceil", 5 }, 8969 },
	{ { (Byte *)"lfloor", 6 }, 8970 }, { { (Byte *)"rfloor", 6 }, 8971 }, { { (Byte *)"lang", 4 }, 9001 },   { { (Byte *)"rang", 4 }, 9002 },
	{ { (Byte *)"loz", 3 }, 9674 },    { { (Byte *)"spades", 6 }, 9824 }, { { (Byte *)"clubs", 4 }, 9827 },  { { (Byte *)"hearts", 6 }, 9829 },
	{ { (Byte *)"diams", 5 }, 9830 },
};

const Int HtmlEntityTableLength = sizeof(HtmlEntityTable) / sizeof(HtmlEntity);
