
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

static const Byte Identity[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup00[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,1  ,0  ,0  ,0  ,2  ,3  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,4  ,0  ,5  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	6  ,7  ,8  ,9  ,10 ,11 ,12 ,13 ,14 ,15 ,16 ,17 ,18 ,19 ,20 ,21 ,
	22 ,23 ,24 ,25 ,26 ,27 ,28 ,29 ,30 ,31 ,32 ,33 ,34 ,35 ,36 ,37 ,
	38 ,39 ,40 ,41 ,42 ,43 ,44 ,45 ,46 ,47 ,48 ,49 ,50 ,51 ,52 ,53 ,
	54 ,55 ,56 ,57 ,58 ,59 ,60 ,61 ,62 ,63 ,64 ,65 ,66 ,67 ,68 ,69 ,
	70 ,71 ,72 ,73 ,74 ,75 ,76 ,77 ,78 ,79 ,80 ,81 ,82 ,83 ,84 ,85 ,
	86 ,87 ,88 ,89 ,90 ,91 ,92 ,93 ,94 ,95 ,96 ,97 ,98 ,99 ,100,101,
};

static const Byte HtmlEntityLookup01[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,102,103,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	104,105,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,106,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,107,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup02[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,108,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,109,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup03[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,
	125,126,0  ,127,128,129,130,131,132,133,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,
	149,150,151,152,153,154,155,156,157,158,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,159,160,0  ,0  ,0  ,161,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup20[] = {
	0  ,0  ,162,163,0  ,0  ,0  ,0  ,0  ,164,0  ,0  ,165,166,167,168,
	0  ,0  ,0  ,169,170,0  ,0  ,0  ,171,172,173,0  ,174,175,176,0  ,
	177,178,179,0  ,0  ,0  ,180,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	181,0  ,182,183,0  ,0  ,0  ,0  ,0  ,184,185,0  ,0  ,0  ,186,0  ,
	0  ,0  ,0  ,0  ,187,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,188,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup21[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,189,0  ,0  ,0  ,0  ,0  ,0  ,190,0  ,0  ,0  ,191,0  ,0  ,0  ,
	0  ,0  ,192,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,193,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	194,195,196,197,198,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,199,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	200,201,202,203,204,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup22[] = {
	205,0  ,206,207,0  ,208,0  ,209,210,211,0  ,212,0  ,0  ,0  ,213,
	0  ,214,215,0  ,0  ,0  ,0  ,216,0  ,0  ,217,0  ,0  ,218,219,0  ,
	220,0  ,0  ,0  ,0  ,0  ,0  ,221,222,223,224,225,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,226,0  ,0  ,0  ,0  ,0  ,0  ,0  ,227,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,228,0  ,0  ,229,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	230,231,0  ,0  ,232,233,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,234,235,236,0  ,237,238,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,239,0  ,240,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,241,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,242,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,243,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup23[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,244,245,246,247,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,248,249,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup25[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,250,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte HtmlEntityLookup26[] = {
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	251,0  ,0  ,252,0  ,253,254,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
	0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,0  ,
};

static const Byte *HtmlEntityLookup[64] = {
	HtmlEntityLookup00, HtmlEntityLookup01, HtmlEntityLookup02, HtmlEntityLookup03,
	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,

	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,

	HtmlEntityLookup20, HtmlEntityLookup21, HtmlEntityLookup22, HtmlEntityLookup23,
	Identity, HtmlEntityLookup25, HtmlEntityLookup26, Identity,
	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,

	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,
	Identity, Identity, Identity, Identity,
};

static const Int HtmlEntityLookupLength = 64;

/// <summary>
/// Convert the given HTML entity value to its common entity name.  This is auto-generated, and uses tables
/// that produce the output quickly without a huge amount of memory overhead.
/// </summary>
/// <param name="name">The value of the HTML entity to look up.</param>
/// <returns>The name for that entity, if it is an entity with a name, or NULL if there is no such entity.</returns>
String HtmlEntityValueToName(Int32 value)
{
	const HtmlEntity *htmlEntity;
	Int lookupIndex;

	if (value >= (HtmlEntityLookupLength << 8) || value < 0) return NULL;

	lookupIndex = HtmlEntityLookup[value >> 8][value & 0xFF];
	if (lookupIndex == 0) return NULL;

	htmlEntity = &HtmlEntityTable[lookupIndex - 1];

	return (String)(&htmlEntity->string);
}

/// <summary>
/// Convert the given name to its HTML entity value.  This is AUTO-GENERATED, and uses an optimized
/// decision tree that runs in O(n) time to convert the string as quickly as possible.
/// </summary>
/// <param name="name">The name of the HTML entity to look up.</param>
/// <returns>The value for that entity, if it is an entity with a value, or -1 if there is no such entity.</returns>
Int32 HtmlEntityNameToValue(String name)
{
	Byte ch;
	const Byte *src;
	Int length;

	if (String_IsNullOrEmpty(name)) return -1;

	src = (const Byte *)String_GetBytes(name);
	length = String_Length(name);

	if (length-- <= 0) return -1;
	switch (*src++) {
		case 'A':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'E':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 198;
					break;
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 193;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 194;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 192;
					break;
				case 'l':
					if (length-- >= 0 && *src++ == 'p')
						if (length-- >= 0 && *src++ == 'h')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 913;
					break;
				case 'r':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'n')
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 197;
					break;
				case 't':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'l')
							if (length-- >= 0 && *src++ == 'd')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 195;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 196;
					break;
			}
			break;
		case 'B':
			if (length-- >= 0 && *src++ == 'e')
				if (length-- >= 0 && *src++ == 't')
					if (length-- >= 0 && *src++ == 'a')
						if (length <= 0) return 914;
			break;
		case 'C':
			if (length-- <= 0) return -1;
			if ((ch = *src++) == 'c') {
				if (length-- >= 0 && *src++ == 'e')
					if (length-- >= 0 && *src++ == 'd')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'l')
								if (length <= 0) return 199;
			}
			else if (ch == 'h') {
				if (length-- >= 0 && *src++ == 'i')
					if (length <= 0) return 935;
			}
			break;
		case 'D':
			if (length-- <= 0) return -1;
			if ((ch = *src++) == 'a') {
				if (length-- >= 0 && *src++ == 'g')
					if (length-- >= 0 && *src++ == 'g')
						if (length-- >= 0 && *src++ == 'e')
							if (length-- >= 0 && *src++ == 'r')
								if (length <= 0) return 8225;
			}
			else if (ch == 'e') {
				if (length-- >= 0 && *src++ == 'l')
					if (length-- >= 0 && *src++ == 't')
						if (length-- >= 0 && *src++ == 'a')
							if (length <= 0) return 916;
			}
			break;
		case 'E':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'T':
					if (length-- >= 0 && *src++ == 'H')
						if (length <= 0) return 208;
					break;
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 201;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 202;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 200;
					break;
				case 'p':
					if (length-- >= 0 && *src++ == 's')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'l')
								if (length-- >= 0 && *src++ == 'o')
									if (length-- >= 0 && *src++ == 'n')
										if (length <= 0) return 917;
					break;
				case 't':
					if (length-- >= 0 && *src++ == 'a')
						if (length <= 0) return 919;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 203;
					break;
			}
			break;
		case 'G':
			if (length-- >= 0 && *src++ == 'a')
				if (length-- >= 0 && *src++ == 'm')
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'a')
							if (length <= 0) return 915;
			break;
		case 'I':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 205;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 206;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 204;
					break;
				case 'o':
					if (length-- >= 0 && *src++ == 't')
						if (length-- >= 0 && *src++ == 'a')
							if (length <= 0) return 921;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 207;
					break;
			}
			break;
		case 'K':
			if (length-- >= 0 && *src++ == 'a')
				if (length-- >= 0 && *src++ == 'p')
					if (length-- >= 0 && *src++ == 'p')
						if (length-- >= 0 && *src++ == 'a')
							if (length <= 0) return 922;
			break;
		case 'L':
			if (length-- >= 0 && *src++ == 'a')
				if (length-- >= 0 && *src++ == 'm')
					if (length-- >= 0 && *src++ == 'b')
						if (length-- >= 0 && *src++ == 'd')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 923;
			break;
		case 'M':
			if (length-- >= 0 && *src++ == 'u')
				if (length <= 0) return 924;
			break;
		case 'N':
			if (length-- <= 0) return -1;
			if ((ch = *src++) == 't') {
				if (length-- >= 0 && *src++ == 'i')
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'd')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 209;
			}
			else if (ch == 'u') {
				if (length <= 0) return 925;
			}
			break;
		case 'O':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'E':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 338;
					break;
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 211;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 212;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 210;
					break;
				case 'm':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'e') {
						if (length-- >= 0 && *src++ == 'g')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 937;
					}
					else if (ch == 'i') {
						if (length-- >= 0 && *src++ == 'c')
							if (length-- >= 0 && *src++ == 'r')
								if (length-- >= 0 && *src++ == 'o')
									if (length-- >= 0 && *src++ == 'n')
										if (length <= 0) return 927;
					}
					break;
				case 's':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 's')
								if (length-- >= 0 && *src++ == 'h')
									if (length <= 0) return 216;
					break;
				case 't':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'l')
							if (length-- >= 0 && *src++ == 'd')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 213;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 214;
					break;
			}
			break;
		case 'P':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'h':
					if (length-- >= 0 && *src++ == 'i')
						if (length <= 0) return 934;
					break;
				case 'i':
					if (length <= 0) return 928;
					break;
				case 'r':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'm')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 8243;
					break;
				case 's':
					if (length-- >= 0 && *src++ == 'i')
						if (length <= 0) return 936;
					break;
			}
			break;
		case 'R':
			if (length-- >= 0 && *src++ == 'h')
				if (length-- >= 0 && *src++ == 'o')
					if (length <= 0) return 929;
			break;
		case 'S':
			if (length-- <= 0) return -1;
			if ((ch = *src++) == 'c') {
				if (length-- >= 0 && *src++ == 'a')
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'o')
							if (length-- >= 0 && *src++ == 'n')
								if (length <= 0) return 352;
			}
			else if (ch == 'i') {
				if (length-- >= 0 && *src++ == 'g')
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'a')
							if (length <= 0) return 931;
			}
			break;
		case 'T':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'H':
					if (length-- >= 0 && *src++ == 'O')
						if (length-- >= 0 && *src++ == 'R')
							if (length-- >= 0 && *src++ == 'N')
								if (length <= 0) return 222;
					break;
				case 'a':
					if (length-- >= 0 && *src++ == 'u')
						if (length <= 0) return 932;
					break;
				case 'h':
					if (length-- >= 0 && *src++ == 'e')
						if (length-- >= 0 && *src++ == 't')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 920;
					break;
			}
			break;
		case 'U':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 218;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 219;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 217;
					break;
				case 'p':
					if (length-- >= 0 && *src++ == 's')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'l')
								if (length-- >= 0 && *src++ == 'o')
									if (length-- >= 0 && *src++ == 'n')
										if (length <= 0) return 933;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 220;
					break;
			}
			break;
		case 'X':
			if (length-- >= 0 && *src++ == 'i')
				if (length <= 0) return 926;
			break;
		case 'Y':
			if (length-- <= 0) return -1;
			if ((ch = *src++) == 'a') {
				if (length-- >= 0 && *src++ == 'c')
					if (length-- >= 0 && *src++ == 'u')
						if (length-- >= 0 && *src++ == 't')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 221;
			}
			else if (ch == 'u') {
				if (length-- >= 0 && *src++ == 'm')
					if (length-- >= 0 && *src++ == 'l')
						if (length <= 0) return 376;
			}
			break;
		case 'Z':
			if (length-- >= 0 && *src++ == 'e')
				if (length-- >= 0 && *src++ == 't')
					if (length-- >= 0 && *src++ == 'a')
						if (length <= 0) return 918;
			break;
		case 'a':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 225;
					break;
				case 'c':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'i') {
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 226;
					}
					else if (ch == 'u') {
						if (length-- >= 0 && *src++ == 't')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 180;
					}
					break;
				case 'e':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 230;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 224;
					break;
				case 'l':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'e') {
						if (length-- >= 0 && *src++ == 'f')
							if (length-- >= 0 && *src++ == 's')
								if (length-- >= 0 && *src++ == 'y')
									if (length-- >= 0 && *src++ == 'm')
										if (length <= 0) return 8501;
					}
					else if (ch == 'p') {
						if (length-- >= 0 && *src++ == 'h')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 945;
					}
					break;
				case 'm':
					if (length-- >= 0 && *src++ == 'p')
						if (length <= 0) return 38;
					break;
				case 'n':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'd') {
						if (length <= 0) return 8743;
					}
					else if (ch == 'g') {
						if (length <= 0) return 8736;
					}
					break;
				case 'p':
					if (length-- >= 0 && *src++ == 'o')
						if (length-- >= 0 && *src++ == 's')
							if (length <= 0) return 39;
					break;
				case 'r':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'n')
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 229;
					break;
				case 's':
					if (length-- >= 0 && *src++ == 'y')
						if (length-- >= 0 && *src++ == 'm')
							if (length-- >= 0 && *src++ == 'p')
								if (length <= 0) return 8776;
					break;
				case 't':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'l')
							if (length-- >= 0 && *src++ == 'd')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 227;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 228;
					break;
			}
			break;
		case 'b':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'd':
					if (length-- >= 0 && *src++ == 'q')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 'o')
								if (length <= 0) return 8222;
					break;
				case 'e':
					if (length-- >= 0 && *src++ == 't')
						if (length-- >= 0 && *src++ == 'a')
							if (length <= 0) return 946;
					break;
				case 'r':
					if (length-- >= 0 && *src++ == 'v')
						if (length-- >= 0 && *src++ == 'b')
							if (length-- >= 0 && *src++ == 'a')
								if (length-- >= 0 && *src++ == 'r')
									if (length <= 0) return 166;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 8226;
					break;
			}
			break;
		case 'c':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'p')
						if (length <= 0) return 8745;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'e')
						if (length-- >= 0 && *src++ == 'd')
							if (length-- >= 0 && *src++ == 'i')
								if (length-- >= 0 && *src++ == 'l')
									if (length <= 0) return 231;
					break;
				case 'e':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'd') {
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'l')
								if (length <= 0) return 184;
					}
					else if (ch == 'n') {
						if (length-- >= 0 && *src++ == 't')
							if (length <= 0) return 162;
					}
					break;
				case 'h':
					if (length-- >= 0 && *src++ == 'i')
						if (length <= 0) return 967;
					break;
				case 'i':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'c')
							if (length <= 0) return 710;
					break;
				case 'l':
					if (length-- >= 0 && *src++ == 'u')
						if (length-- >= 0 && *src++ == 'b')
							if (length-- >= 0 && *src++ == 's')
								if (length <= 0) return 9827;
					break;
				case 'o':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'n') {
						if (length-- >= 0 && *src++ == 'g')
							if (length <= 0) return 8773;
					}
					else if (ch == 'p') {
						if (length-- >= 0 && *src++ == 'y')
							if (length <= 0) return 169;
					}
					break;
				case 'r':
					if (length-- >= 0 && *src++ == 'a')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'r')
								if (length <= 0) return 8629;
					break;
				case 'u':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'p') {
						if (length <= 0) return 8746;
					}
					else if (ch == 'r') {
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'e')
								if (length-- >= 0 && *src++ == 'n')
									if (length <= 0) return 164;
					}
					break;
			}
			break;
		case 'd':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'A':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 8659;
					break;
				case 'a':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'g') {
						if (length-- >= 0 && *src++ == 'g')
							if (length-- >= 0 && *src++ == 'e')
								if (length-- >= 0 && *src++ == 'r')
									if (length <= 0) return 8224;
					}
					else if (ch == 'r') {
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 8595;
					}
					break;
				case 'e':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'g') {
						if (length <= 0) return 176;
					}
					else if (ch == 'l') {
						if (length-- >= 0 && *src++ == 't')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 948;
					}
					break;
				case 'i':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'a') {
						if (length-- >= 0 && *src++ == 'm')
							if (length-- >= 0 && *src++ == 's')
								if (length <= 0) return 9830;
					}
					else if (ch == 'v') {
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'd')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 247;
					}
					break;
			}
			break;
		case 'e':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 233;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 234;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 232;
					break;
				case 'm':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'p') {
						if (length-- >= 0 && *src++ == 't')
							if (length-- >= 0 && *src++ == 'y')
								if (length <= 0) return 8709;
					}
					else if (ch == 's') {
						if (length-- >= 0 && *src++ == 'p')
							if (length <= 0) return 8195;
					}
					break;
				case 'n':
					if (length-- >= 0 && *src++ == 's')
						if (length-- >= 0 && *src++ == 'p')
							if (length <= 0) return 8194;
					break;
				case 'p':
					if (length-- >= 0 && *src++ == 's')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'l')
								if (length-- >= 0 && *src++ == 'o')
									if (length-- >= 0 && *src++ == 'n')
										if (length <= 0) return 949;
					break;
				case 'q':
					if (length-- >= 0 && *src++ == 'u')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'v')
								if (length <= 0) return 8801;
					break;
				case 't':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'a') {
						if (length <= 0) return 951;
					}
					else if (ch == 'h') {
						if (length <= 0) return 240;
					}
					break;
				case 'u':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'm') {
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 235;
					}
					else if (ch == 'r') {
						if (length-- >= 0 && *src++ == 'o')
							if (length <= 0) return 8364;
					}
					break;
				case 'x':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 's')
							if (length-- >= 0 && *src++ == 't')
								if (length <= 0) return 8707;
					break;
			}
			break;
		case 'f':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'n':
					if (length-- >= 0 && *src++ == 'o')
						if (length-- >= 0 && *src++ == 'f')
							if (length <= 0) return 402;
					break;
				case 'o':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'l')
								if (length-- >= 0 && *src++ == 'l')
									if (length <= 0) return 8704;
					break;
				case 'r':
					if (length-- >= 0 && *src++ == 'a') {
						if (length-- <= 0) return -1;
						if ((ch = *src++) == 'c') {
							if (length-- <= 0) return -1;
							if ((ch = *src++) == '1') {
								if (length-- <= 0) return -1;
								if ((ch = *src++) == '2') {
									if (length <= 0) return 189;
								}
								else if (ch == '4') {
									if (length <= 0) return 188;
								}
							}
							else if (ch == '3') {
								if (length-- >= 0 && *src++ == '4')
									if (length <= 0) return 190;
							}
						}
						else if (ch == 's') {
							if (length-- >= 0 && *src++ == 'l')
								if (length <= 0) return 8260;
						}
					}
					break;
			}
			break;
		case 'g':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'm')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 947;
					break;
				case 'e':
					if (length <= 0) return 8805;
					break;
				case 't':
					if (length <= 0) return 62;
					break;
			}
			break;
		case 'h':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'A':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 8660;
					break;
				case 'a':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 8596;
					break;
				case 'e':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'a') {
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 's')
									if (length <= 0) return 9829;
					}
					else if (ch == 'l') {
						if (length-- >= 0 && *src++ == 'l')
							if (length-- >= 0 && *src++ == 'i')
								if (length-- >= 0 && *src++ == 'p')
									if (length <= 0) return 8230;
					}
					break;
			}
			break;
		case 'i':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 237;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 238;
					break;
				case 'e':
					if (length-- >= 0 && *src++ == 'x')
						if (length-- >= 0 && *src++ == 'c')
							if (length-- >= 0 && *src++ == 'l')
								if (length <= 0) return 161;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 236;
					break;
				case 'm':
					if (length-- >= 0 && *src++ == 'a')
						if (length-- >= 0 && *src++ == 'g')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 8465;
					break;
				case 'n':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'f') {
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'n')
								if (length <= 0) return 8734;
					}
					else if (ch == 't') {
						if (length <= 0) return 8747;
					}
					break;
				case 'o':
					if (length-- >= 0 && *src++ == 't')
						if (length-- >= 0 && *src++ == 'a')
							if (length <= 0) return 953;
					break;
				case 'q':
					if (length-- >= 0 && *src++ == 'u')
						if (length-- >= 0 && *src++ == 'e')
							if (length-- >= 0 && *src++ == 's')
								if (length-- >= 0 && *src++ == 't')
									if (length <= 0) return 191;
					break;
				case 's':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'n')
							if (length <= 0) return 8712;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 239;
					break;
			}
			break;
		case 'k':
			if (length-- >= 0 && *src++ == 'a')
				if (length-- >= 0 && *src++ == 'p')
					if (length-- >= 0 && *src++ == 'p')
						if (length-- >= 0 && *src++ == 'a')
							if (length <= 0) return 954;
			break;
		case 'l':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'A':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 8656;
					break;
				case 'a':
					if (length-- <= 0) return -1;
					switch (*src++) {
						case 'm':
							if (length-- >= 0 && *src++ == 'b')
								if (length-- >= 0 && *src++ == 'd')
									if (length-- >= 0 && *src++ == 'a')
										if (length <= 0) return 955;
							break;
						case 'n':
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 9001;
							break;
						case 'q':
							if (length-- >= 0 && *src++ == 'u')
								if (length-- >= 0 && *src++ == 'o')
									if (length <= 0) return 171;
							break;
						case 'r':
							if (length-- >= 0 && *src++ == 'r')
								if (length <= 0) return 8592;
							break;
					}
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'e')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'l')
								if (length <= 0) return 8968;
					break;
				case 'd':
					if (length-- >= 0 && *src++ == 'q')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 'o')
								if (length <= 0) return 8220;
					break;
				case 'e':
					if (length <= 0) return 8804;
					break;
				case 'f':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'o')
							if (length-- >= 0 && *src++ == 'o')
								if (length-- >= 0 && *src++ == 'r')
									if (length <= 0) return 8970;
					break;
				case 'o':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'w') {
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 's')
								if (length-- >= 0 && *src++ == 't')
									if (length <= 0) return 8727;
					}
					else if (ch == 'z') {
						if (length <= 0) return 9674;
					}
					break;
				case 'r':
					if (length-- >= 0 && *src++ == 'm')
						if (length <= 0) return 8206;
					break;
				case 's':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'a') {
						if (length-- >= 0 && *src++ == 'q')
							if (length-- >= 0 && *src++ == 'u')
								if (length-- >= 0 && *src++ == 'o')
									if (length <= 0) return 8249;
					}
					else if (ch == 'q') {
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 'o')
								if (length <= 0) return 8216;
					}
					break;
				case 't':
					if (length <= 0) return 60;
					break;
			}
			break;
		case 'm':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 175;
					break;
				case 'd':
					if (length-- >= 0 && *src++ == 'a')
						if (length-- >= 0 && *src++ == 's')
							if (length-- >= 0 && *src++ == 'h')
								if (length <= 0) return 8212;
					break;
				case 'i':
					if (length-- <= 0) return -1;
					switch (*src++) {
						case 'c':
							if (length-- >= 0 && *src++ == 'r')
								if (length-- >= 0 && *src++ == 'o')
									if (length <= 0) return 181;
							break;
						case 'd':
							if (length-- >= 0 && *src++ == 'd')
								if (length-- >= 0 && *src++ == 'o')
									if (length-- >= 0 && *src++ == 't')
										if (length <= 0) return 183;
							break;
						case 'n':
							if (length-- >= 0 && *src++ == 'u')
								if (length-- >= 0 && *src++ == 's')
									if (length <= 0) return 8722;
							break;
					}
					break;
				case 'u':
					if (length <= 0) return 956;
					break;
			}
			break;
		case 'n':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'b')
						if (length-- >= 0 && *src++ == 'l')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 8711;
					break;
				case 'b':
					if (length-- >= 0 && *src++ == 's')
						if (length-- >= 0 && *src++ == 'p')
							if (length <= 0) return 160;
					break;
				case 'd':
					if (length-- >= 0 && *src++ == 'a')
						if (length-- >= 0 && *src++ == 's')
							if (length-- >= 0 && *src++ == 'h')
								if (length <= 0) return 8211;
					break;
				case 'e':
					if (length <= 0) return 8800;
					break;
				case 'i':
					if (length <= 0) return 8715;
					break;
				case 'o':
					if (length-- >= 0 && *src++ == 't') {
						if (length <= 0) return 172;
					}
					break;
				case 's':
					if (length-- >= 0 && *src++ == 'u')
						if (length-- >= 0 && *src++ == 'b')
							if (length <= 0) return 8836;
					break;
				case 't':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'l')
							if (length-- >= 0 && *src++ == 'd')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 241;
					break;
				case 'u':
					if (length <= 0) return 957;
					break;
			}
			break;
		case 'o':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 243;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 244;
					break;
				case 'e':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 339;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 242;
					break;
				case 'l':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'n')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 8254;
					break;
				case 'm':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'e') {
						if (length-- >= 0 && *src++ == 'g')
							if (length-- >= 0 && *src++ == 'a')
								if (length <= 0) return 969;
					}
					else if (ch == 'i') {
						if (length-- >= 0 && *src++ == 'c')
							if (length-- >= 0 && *src++ == 'r')
								if (length-- >= 0 && *src++ == 'o')
									if (length-- >= 0 && *src++ == 'n')
										if (length <= 0) return 959;
					}
					break;
				case 'p':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 's')
								if (length <= 0) return 8853;
					break;
				case 'r':
					if (length <= 0) return 8744;
					break;
				case 's':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 's')
								if (length-- >= 0 && *src++ == 'h')
									if (length <= 0) return 248;
					break;
				case 't':
					if (length-- >= 0 && *src++ == 'i') {
						if (length-- <= 0) return -1;
						if ((ch = *src++) == 'l') {
							if (length-- >= 0 && *src++ == 'd')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 245;
						}
						else if (ch == 'm') {
							if (length-- >= 0 && *src++ == 'e')
								if (length-- >= 0 && *src++ == 's')
									if (length <= 0) return 8855;
						}
					}
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 246;
					break;
			}
			break;
		case 'p':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'r') {
						if (length-- <= 0) return -1;
						if ((ch = *src++) == 'a') {
							if (length <= 0) return 182;
						}
						else if (ch == 't') {
							if (length <= 0) return 8706;
						}
					}
					break;
				case 'e':
					if (length-- >= 0 && *src++ == 'r') {
						if (length-- <= 0) return -1;
						if ((ch = *src++) == 'm') {
							if (length-- >= 0 && *src++ == 'i')
								if (length-- >= 0 && *src++ == 'l')
									if (length <= 0) return 8240;
						}
						else if (ch == 'p') {
							if (length <= 0) return 8869;
						}
					}
					break;
				case 'h':
					if (length-- >= 0 && *src++ == 'i')
						if (length <= 0) return 966;
					break;
				case 'i':
					if (length <= 0) return 960;
					break;
				case 'l':
					if (length-- >= 0 && *src++ == 'u')
						if (length-- >= 0 && *src++ == 's')
							if (length-- >= 0 && *src++ == 'm')
								if (length-- >= 0 && *src++ == 'n')
									if (length <= 0) return 177;
					break;
				case 'o':
					if (length-- >= 0 && *src++ == 'u')
						if (length-- >= 0 && *src++ == 'n')
							if (length-- >= 0 && *src++ == 'd')
								if (length <= 0) return 163;
					break;
				case 'r':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'i') {
						if (length-- >= 0 && *src++ == 'm')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 8242;
					}
					else if (ch == 'o') {
						if (length-- <= 0) return -1;
						if ((ch = *src++) == 'd') {
							if (length <= 0) return 8719;
						}
						else if (ch == 'p') {
							if (length <= 0) return 8733;
						}
					}
					break;
				case 's':
					if (length-- >= 0 && *src++ == 'i')
						if (length <= 0) return 968;
					break;
			}
			break;
		case 'q':
			if (length-- >= 0 && *src++ == 'u')
				if (length-- >= 0 && *src++ == 'o')
					if (length-- >= 0 && *src++ == 't')
						if (length <= 0) return 34;
			break;
		case 'r':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'A':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 8658;
					break;
				case 'a':
					if (length-- <= 0) return -1;
					switch (*src++) {
						case 'd':
							if (length-- >= 0 && *src++ == 'i')
								if (length-- >= 0 && *src++ == 'c')
									if (length <= 0) return 8730;
							break;
						case 'n':
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 9002;
							break;
						case 'q':
							if (length-- >= 0 && *src++ == 'u')
								if (length-- >= 0 && *src++ == 'o')
									if (length <= 0) return 187;
							break;
						case 'r':
							if (length-- >= 0 && *src++ == 'r')
								if (length <= 0) return 8594;
							break;
					}
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'e')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'l')
								if (length <= 0) return 8969;
					break;
				case 'd':
					if (length-- >= 0 && *src++ == 'q')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 'o')
								if (length <= 0) return 8221;
					break;
				case 'e':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'a') {
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 8476;
					}
					else if (ch == 'g') {
						if (length <= 0) return 174;
					}
					break;
				case 'f':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'o')
							if (length-- >= 0 && *src++ == 'o')
								if (length-- >= 0 && *src++ == 'r')
									if (length <= 0) return 8971;
					break;
				case 'h':
					if (length-- >= 0 && *src++ == 'o')
						if (length <= 0) return 961;
					break;
				case 'l':
					if (length-- >= 0 && *src++ == 'm')
						if (length <= 0) return 8207;
					break;
				case 's':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'a') {
						if (length-- >= 0 && *src++ == 'q')
							if (length-- >= 0 && *src++ == 'u')
								if (length-- >= 0 && *src++ == 'o')
									if (length <= 0) return 8250;
					}
					else if (ch == 'q') {
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 'o')
								if (length <= 0) return 8217;
					}
					break;
			}
			break;
		case 's':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'b':
					if (length-- >= 0 && *src++ == 'q')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 'o')
								if (length <= 0) return 8218;
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'a')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'o')
								if (length-- >= 0 && *src++ == 'n')
									if (length <= 0) return 353;
					break;
				case 'd':
					if (length-- >= 0 && *src++ == 'o')
						if (length-- >= 0 && *src++ == 't')
							if (length <= 0) return 8901;
					break;
				case 'e':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 't')
							if (length <= 0) return 167;
					break;
				case 'h':
					if (length-- >= 0 && *src++ == 'y')
						if (length <= 0) return 173;
					break;
				case 'i':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'g') {
						if (length-- >= 0 && *src++ == 'm') {
							if (length-- >= 0 && *src++ == 'a') {
								if (length <= 0) return 963;
							}
						}
					}
					else if (ch == 'm') {
						if (length <= 0) return 8764;
					}
					break;
				case 'p':
					if (length-- >= 0 && *src++ == 'a')
						if (length-- >= 0 && *src++ == 'd')
							if (length-- >= 0 && *src++ == 'e')
								if (length-- >= 0 && *src++ == 's')
									if (length <= 0) return 9824;
					break;
				case 'u':
					if (length-- <= 0) return -1;
					switch (*src++) {
						case 'b':
							if (length <= 0) return 8834;
							break;
						case 'm':
							if (length <= 0) return 8721;
							break;
						case 'p':
							if (length <= 0) return 8835;
							break;
					}
					break;
				case 'z':
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'g')
								if (length <= 0) return 223;
					break;
			}
			break;
		case 't':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'u')
						if (length <= 0) return 964;
					break;
				case 'h':
					if (length-- <= 0) return -1;
					switch (*src++) {
						case 'e':
							if (length-- <= 0) return -1;
							if ((ch = *src++) == 'r') {
								if (length-- >= 0 && *src++ == 'e')
									if (length-- >= 0 && *src++ == '4')
										if (length <= 0) return 8756;
							}
							else if (ch == 't') {
								if (length-- >= 0 && *src++ == 'a') {
									if (length <= 0) return 952;
								}
							}
							break;
						case 'i':
							if (length-- >= 0 && *src++ == 'n')
								if (length-- >= 0 && *src++ == 's')
									if (length-- >= 0 && *src++ == 'p')
										if (length <= 0) return 8201;
							break;
						case 'o':
							if (length-- >= 0 && *src++ == 'r')
								if (length-- >= 0 && *src++ == 'n')
									if (length <= 0) return 254;
							break;
					}
					break;
				case 'i':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'l') {
						if (length-- >= 0 && *src++ == 'd')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 732;
					}
					else if (ch == 'm') {
						if (length-- >= 0 && *src++ == 'e')
							if (length-- >= 0 && *src++ == 's')
								if (length <= 0) return 215;
					}
					break;
				case 'r':
					if (length-- >= 0 && *src++ == 'a')
						if (length-- >= 0 && *src++ == 'd')
							if (length-- >= 0 && *src++ == 'e')
								if (length <= 0) return 8482;
					break;
			}
			break;
		case 'u':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'A':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 8657;
					break;
				case 'a':
					if (length-- <= 0) return -1;
					if ((ch = *src++) == 'c') {
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 250;
					}
					else if (ch == 'r') {
						if (length-- >= 0 && *src++ == 'r')
							if (length <= 0) return 8593;
					}
					break;
				case 'c':
					if (length-- >= 0 && *src++ == 'i')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'c')
								if (length <= 0) return 251;
					break;
				case 'g':
					if (length-- >= 0 && *src++ == 'r')
						if (length-- >= 0 && *src++ == 'a')
							if (length-- >= 0 && *src++ == 'v')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 249;
					break;
				case 'm':
					if (length-- >= 0 && *src++ == 'l')
						if (length <= 0) return 168;
					break;
				case 'p':
					if (length-- >= 0 && *src++ == 's') {
						if (length-- >= 0 && *src++ == 'i') {
							if (length-- <= 0) return -1;
							if ((ch = *src++) == 'h') {
								if (length <= 0) return 978;
							}
							else if (ch == 'l') {
								if (length-- >= 0 && *src++ == 'o')
									if (length-- >= 0 && *src++ == 'n')
										if (length <= 0) return 965;
							}
						}
					}
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 252;
					break;
			}
			break;
		case 'v':
			if (length-- >= 0 && *src++ == 'e')
				if (length-- >= 0 && *src++ == 'l')
					if (length-- >= 0 && *src++ == 'l')
						if (length-- >= 0 && *src++ == 'i')
							if (length-- >= 0 && *src++ == 'p')
								if (length <= 0) return 8942;
			break;
		case 'w':
			if (length-- >= 0 && *src++ == 'e')
				if (length-- >= 0 && *src++ == 'i')
					if (length-- >= 0 && *src++ == 'e')
						if (length-- >= 0 && *src++ == 'r')
							if (length-- >= 0 && *src++ == 'p')
								if (length <= 0) return 8472;
			break;
		case 'x':
			if (length-- >= 0 && *src++ == 'i')
				if (length <= 0) return 958;
			break;
		case 'y':
			if (length-- <= 0) return -1;
			switch (*src++) {
				case 'a':
					if (length-- >= 0 && *src++ == 'c')
						if (length-- >= 0 && *src++ == 'u')
							if (length-- >= 0 && *src++ == 't')
								if (length-- >= 0 && *src++ == 'e')
									if (length <= 0) return 253;
					break;
				case 'e':
					if (length-- >= 0 && *src++ == 'n')
						if (length <= 0) return 165;
					break;
				case 'u':
					if (length-- >= 0 && *src++ == 'm')
						if (length-- >= 0 && *src++ == 'l')
							if (length <= 0) return 255;
					break;
			}
			break;
		case 'z':
			if (length-- <= 0) return -1;
			if ((ch = *src++) == 'e') {
				if (length-- >= 0 && *src++ == 't')
					if (length-- >= 0 && *src++ == 'a')
						if (length <= 0) return 950;
			}
			else if (ch == 'w') {
				if (length-- <= 0) return -1;
				if ((ch = *src++) == 'j') {
					if (length <= 0) return 8205;
				}
				else if (ch == 'n') {
					if (length-- >= 0 && *src++ == 'j')
						if (length <= 0) return 8204;
				}
			}
			break;
	}
	return -1;
}
