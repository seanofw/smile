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
#include <smile/internal/staticstring.h>

STATIC_STRING(_quot, "quot");
STATIC_STRING(_amp, "amp");
STATIC_STRING(_apos, "apos");
STATIC_STRING(_lt, "lt");
STATIC_STRING(_gt, "gt");
STATIC_STRING(_nbsp, "nbsp");
STATIC_STRING(_iexcl, "iexcl");
STATIC_STRING(_cent, "cent");
STATIC_STRING(_pound, "pound");
STATIC_STRING(_curren, "curren");
STATIC_STRING(_yen, "yen");
STATIC_STRING(_brvbar, "brvbar");
STATIC_STRING(_sect, "sect");
STATIC_STRING(_uml, "uml");
STATIC_STRING(_copy, "copy");
STATIC_STRING(_ordf, "ordf");
STATIC_STRING(_laquo, "laquo");
STATIC_STRING(_not, "not");
STATIC_STRING(_shy, "shy");
STATIC_STRING(_reg, "reg");
STATIC_STRING(_macr, "macr");
STATIC_STRING(_deg, "deg");
STATIC_STRING(_plusmn, "plusmn");
STATIC_STRING(_sup2, "sup2");
STATIC_STRING(_sup3, "sup3");
STATIC_STRING(_acute, "acute");
STATIC_STRING(_micro, "micro");
STATIC_STRING(_para, "para");
STATIC_STRING(_middot, "middot");
STATIC_STRING(_cedil, "cedil");
STATIC_STRING(_sup1, "sup1");
STATIC_STRING(_ordm, "ordm");
STATIC_STRING(_raquo, "raquo");
STATIC_STRING(_frac14, "frac14");
STATIC_STRING(_frac12, "frac12");
STATIC_STRING(_frac34, "frac34");
STATIC_STRING(_iquest, "iquest");
STATIC_STRING(_Agrave, "Agrave");
STATIC_STRING(_Aacute, "Aacute");
STATIC_STRING(_Acirc, "Acirc");
STATIC_STRING(_Atilde, "Atilde");
STATIC_STRING(_Auml, "Auml");
STATIC_STRING(_Aring, "Aring");
STATIC_STRING(_AElig, "AElig");
STATIC_STRING(_Ccedil, "Ccedil");
STATIC_STRING(_Egrave, "Egrave");
STATIC_STRING(_Eacute, "Eacute");
STATIC_STRING(_Ecirc, "Ecirc");
STATIC_STRING(_Euml, "Euml");
STATIC_STRING(_Igrave, "Igrave");
STATIC_STRING(_Iacute, "Iacute");
STATIC_STRING(_Icirc, "Icirc");
STATIC_STRING(_Iuml, "Iuml");
STATIC_STRING(_ETH, "ETH");
STATIC_STRING(_Ntilde, "Ntilde");
STATIC_STRING(_Ograve, "Ograve");
STATIC_STRING(_Oacute, "Oacute");
STATIC_STRING(_Ocirc, "Ocirc");
STATIC_STRING(_Otilde, "Otilde");
STATIC_STRING(_Ouml, "Ouml");
STATIC_STRING(_times, "times");
STATIC_STRING(_Oslash, "Oslash");
STATIC_STRING(_Ugrave, "Ugrave");
STATIC_STRING(_Uacute, "Uacute");
STATIC_STRING(_Ucirc, "Ucirc");
STATIC_STRING(_Uuml, "Uuml");
STATIC_STRING(_Yacute, "Yacute");
STATIC_STRING(_THORN, "THORN");
STATIC_STRING(_szlig, "szlig");
STATIC_STRING(_agrave, "agrave");
STATIC_STRING(_aacute, "aacute");
STATIC_STRING(_acirc, "acirc");
STATIC_STRING(_atilde, "atilde");
STATIC_STRING(_auml, "auml");
STATIC_STRING(_aring, "aring");
STATIC_STRING(_aelig, "aelig");
STATIC_STRING(_ccedil, "ccedil");
STATIC_STRING(_egrave, "egrave");
STATIC_STRING(_eacute, "eacute");
STATIC_STRING(_ecirc, "ecirc");
STATIC_STRING(_euml, "euml");
STATIC_STRING(_igrave, "igrave");
STATIC_STRING(_iacute, "iacute");
STATIC_STRING(_icirc, "icirc");
STATIC_STRING(_iuml, "iuml");
STATIC_STRING(_eth, "eth");
STATIC_STRING(_ntilde, "ntilde");
STATIC_STRING(_ograve, "ograve");
STATIC_STRING(_oacute, "oacute");
STATIC_STRING(_ocirc, "ocirc");
STATIC_STRING(_otilde, "otilde");
STATIC_STRING(_ouml, "ouml");
STATIC_STRING(_divide, "divide");
STATIC_STRING(_oslash, "oslash");
STATIC_STRING(_ugrave, "ugrave");
STATIC_STRING(_uacute, "uacute");
STATIC_STRING(_ucirc, "ucirc");
STATIC_STRING(_uuml, "uuml");
STATIC_STRING(_yacute, "yacute");
STATIC_STRING(_thorn, "thorn");
STATIC_STRING(_yuml, "yuml");
STATIC_STRING(_OElig, "OElig");
STATIC_STRING(_oelig, "oelig");
STATIC_STRING(_Scaron, "Scaron");
STATIC_STRING(_scaron, "scaron");
STATIC_STRING(_Yuml, "Yuml");
STATIC_STRING(_fnof, "fnof");
STATIC_STRING(_circ, "circ");
STATIC_STRING(_tilde, "tilde");
STATIC_STRING(_Alpha, "Alpha");
STATIC_STRING(_Beta, "Beta");
STATIC_STRING(_Gamma, "Gamma");
STATIC_STRING(_Delta, "Delta");
STATIC_STRING(_Epsilon, "Epsilon");
STATIC_STRING(_Zeta, "Zeta");
STATIC_STRING(_Eta, "Eta");
STATIC_STRING(_Theta, "Theta");
STATIC_STRING(_Iota, "Iota");
STATIC_STRING(_Kappa, "Kappa");
STATIC_STRING(_Lambda, "Lambda");
STATIC_STRING(_Mu, "Mu");
STATIC_STRING(_Nu, "Nu");
STATIC_STRING(_Xi, "Xi");
STATIC_STRING(_Omicron, "Omicron");
STATIC_STRING(_Pi, "Pi");
STATIC_STRING(_Rho, "Rho");
STATIC_STRING(_Sigma, "Sigma");
STATIC_STRING(_Tau, "Tau");
STATIC_STRING(_Upsilon, "Upsilon");
STATIC_STRING(_Phi, "Phi");
STATIC_STRING(_Chi, "Chi");
STATIC_STRING(_Psi, "Psi");
STATIC_STRING(_Omega, "Omega");
STATIC_STRING(_alpha, "alpha");
STATIC_STRING(_beta, "beta");
STATIC_STRING(_gamma, "gamma");
STATIC_STRING(_delta, "delta");
STATIC_STRING(_epsilon, "epsilon");
STATIC_STRING(_zeta, "zeta");
STATIC_STRING(_eta, "eta");
STATIC_STRING(_theta, "theta");
STATIC_STRING(_iota, "iota");
STATIC_STRING(_kappa, "kappa");
STATIC_STRING(_lambda, "lambda");
STATIC_STRING(_mu, "mu");
STATIC_STRING(_nu, "nu");
STATIC_STRING(_xi, "xi");
STATIC_STRING(_omicron, "omicron");
STATIC_STRING(_pi, "pi");
STATIC_STRING(_rho, "rho");
STATIC_STRING(_sigmaf, "sigmaf");
STATIC_STRING(_sigma, "sigma");
STATIC_STRING(_tau, "tau");
STATIC_STRING(_upsilon, "upsilon");
STATIC_STRING(_phi, "phi");
STATIC_STRING(_chi, "chi");
STATIC_STRING(_psi, "psi");
STATIC_STRING(_omega, "omega");
STATIC_STRING(_thetasym, "thetasym");
STATIC_STRING(_upsih, "upsih");
STATIC_STRING(_piv, "piv");
STATIC_STRING(_ensp, "ensp");
STATIC_STRING(_emsp, "emsp");
STATIC_STRING(_thinsp, "thinsp");
STATIC_STRING(_zwnj, "zwnj");
STATIC_STRING(_zwj, "zwj");
STATIC_STRING(_lrm, "lrm");
STATIC_STRING(_rlm, "rlm");
STATIC_STRING(_ndash, "ndash");
STATIC_STRING(_mdash, "mdash");
STATIC_STRING(_lsquo, "lsquo");
STATIC_STRING(_rsquo, "rsquo");
STATIC_STRING(_sbquo, "sbquo");
STATIC_STRING(_ldquo, "ldquo");
STATIC_STRING(_rdquo, "rdquo");
STATIC_STRING(_bdquo, "bdquo");
STATIC_STRING(_dagger, "dagger");
STATIC_STRING(_Dagger, "Dagger");
STATIC_STRING(_bull, "bull");
STATIC_STRING(_hellip, "hellip");
STATIC_STRING(_permil, "permil");
STATIC_STRING(_prime, "prime");
STATIC_STRING(_Prime, "Prime");
STATIC_STRING(_lsaquo, "lsaquo");
STATIC_STRING(_rsaquo, "rsaquo");
STATIC_STRING(_oline, "oline");
STATIC_STRING(_frasl, "frasl");
STATIC_STRING(_euro, "euro");
STATIC_STRING(_image, "image");
STATIC_STRING(_weierp, "weierp");
STATIC_STRING(_real, "real");
STATIC_STRING(_trade, "trade");
STATIC_STRING(_alefsym, "alefsym");
STATIC_STRING(_larr, "larr");
STATIC_STRING(_uarr, "uarr");
STATIC_STRING(_rarr, "rarr");
STATIC_STRING(_darr, "darr");
STATIC_STRING(_harr, "harr");
STATIC_STRING(_crarr, "crarr");
STATIC_STRING(_lArr, "lArr");
STATIC_STRING(_uArr, "uArr");
STATIC_STRING(_rArr, "rArr");
STATIC_STRING(_dArr, "dArr");
STATIC_STRING(_hArr, "hArr");
STATIC_STRING(_forall, "forall");
STATIC_STRING(_part, "part");
STATIC_STRING(_exist, "exist");
STATIC_STRING(_empty, "empty");
STATIC_STRING(_nabla, "nabla");
STATIC_STRING(_isin, "isin");
STATIC_STRING(_notin, "notin");
STATIC_STRING(_ni, "ni");
STATIC_STRING(_prod, "prod");
STATIC_STRING(_sum, "sum");
STATIC_STRING(_minus, "minus");
STATIC_STRING(_lowast, "lowast");
STATIC_STRING(_radic, "radic");
STATIC_STRING(_prop, "prop");
STATIC_STRING(_infin, "infin");
STATIC_STRING(_ang, "ang");
STATIC_STRING(_and, "and");
STATIC_STRING(_or, "or");
STATIC_STRING(_cap, "cap");
STATIC_STRING(_cup, "cup");
STATIC_STRING(_int, "int");
STATIC_STRING(_there4, "there4");
STATIC_STRING(_sim, "sim");
STATIC_STRING(_cong, "cong");
STATIC_STRING(_asymp, "asymp");
STATIC_STRING(_ne, "ne");
STATIC_STRING(_equiv, "equiv");
STATIC_STRING(_le, "le");
STATIC_STRING(_ge, "ge");
STATIC_STRING(_sub, "sub");
STATIC_STRING(_sup, "sup");
STATIC_STRING(_nsub, "nsub");
STATIC_STRING(_sube, "sube");
STATIC_STRING(_supe, "supe");
STATIC_STRING(_oplus, "oplus");
STATIC_STRING(_otimes, "otimes");
STATIC_STRING(_perp, "perp");
STATIC_STRING(_sdot, "sdot");
STATIC_STRING(_vellip, "vellip");
STATIC_STRING(_lceil, "lceil");
STATIC_STRING(_rceil, "rceil");
STATIC_STRING(_lfloor, "lfloor");
STATIC_STRING(_rfloor, "rfloor");
STATIC_STRING(_lang, "lang");
STATIC_STRING(_rang, "rang");
STATIC_STRING(_loz, "loz");
STATIC_STRING(_spades, "spades");
STATIC_STRING(_clubs, "clubs");
STATIC_STRING(_hearts, "hearts");
STATIC_STRING(_diams, "diams");

const HtmlEntity HtmlEntityTable[] = {
	{ (String)&_quotStruct, 34 },     { (String)&_ampStruct, 38 },      { (String)&_aposStruct, 39 },
	{ (String)&_ltStruct, 60 },       { (String)&_gtStruct, 62 },
			
	{ (String)&_nbspStruct, 160 },    { (String)&_iexclStruct, 161 },   { (String)&_centStruct, 162 },    { (String)&_poundStruct, 163 },
	{ (String)&_currenStruct, 164 },  { (String)&_yenStruct, 165 },     { (String)&_brvbarStruct, 166 },  { (String)&_sectStruct, 167 },
	{ (String)&_umlStruct, 168 },     { (String)&_copyStruct, 169 },    { (String)&_ordfStruct, 170 },    { (String)&_laquoStruct, 171 },
	{ (String)&_notStruct, 172 },     { (String)&_shyStruct, 173 },     { (String)&_regStruct, 174 },     { (String)&_macrStruct, 175 },
	{ (String)&_degStruct, 176 },     { (String)&_plusmnStruct, 177 },  { (String)&_sup2Struct, 178 },    { (String)&_sup3Struct, 179 },
	{ (String)&_acuteStruct, 180 },   { (String)&_microStruct, 181 },   { (String)&_paraStruct, 182 },    { (String)&_middotStruct, 183 },
	{ (String)&_cedilStruct, 184 },   { (String)&_sup1Struct, 185 },    { (String)&_ordmStruct, 186 },    { (String)&_raquoStruct, 187 },
	{ (String)&_frac14Struct, 188 },  { (String)&_frac12Struct, 189 },  { (String)&_frac34Struct, 190 },  { (String)&_iquestStruct, 191 },

	{ (String)&_AgraveStruct, 192 },  { (String)&_AacuteStruct, 193 },  { (String)&_AcircStruct, 194 },   { (String)&_AtildeStruct, 195 },
	{ (String)&_AumlStruct, 196 },    { (String)&_AringStruct, 197 },   { (String)&_AEligStruct, 198 },   { (String)&_CcedilStruct, 199 },
	{ (String)&_EgraveStruct, 200 },  { (String)&_EacuteStruct, 201 },  { (String)&_EcircStruct, 202 },   { (String)&_EumlStruct, 203 },
	{ (String)&_IgraveStruct, 204 },  { (String)&_IacuteStruct, 205 },  { (String)&_IcircStruct, 206 },   { (String)&_IumlStruct, 207 },
	{ (String)&_ETHStruct, 208 },     { (String)&_NtildeStruct, 209 },  { (String)&_OgraveStruct, 210 },  { (String)&_OacuteStruct, 211 },
	{ (String)&_OcircStruct, 212 },   { (String)&_OtildeStruct, 213 },  { (String)&_OumlStruct, 214 },    { (String)&_timesStruct, 215 },
	{ (String)&_OslashStruct, 216 },  { (String)&_UgraveStruct, 217 },  { (String)&_UacuteStruct, 218 },  { (String)&_UcircStruct, 219 },
	{ (String)&_UumlStruct, 220 },    { (String)&_YacuteStruct, 221 },  { (String)&_THORNStruct, 222 },   { (String)&_szligStruct, 223 },

	{ (String)&_agraveStruct, 224 },  { (String)&_aacuteStruct, 225 },  { (String)&_acircStruct, 226 },   { (String)&_atildeStruct, 227 },
	{ (String)&_aumlStruct, 228 },    { (String)&_aringStruct, 229 },   { (String)&_aeligStruct, 230 },   { (String)&_ccedilStruct, 231 },
	{ (String)&_egraveStruct, 232 },  { (String)&_eacuteStruct, 233 },  { (String)&_ecircStruct, 234 },   { (String)&_eumlStruct, 235 },
	{ (String)&_igraveStruct, 236 },  { (String)&_iacuteStruct, 237 },  { (String)&_icircStruct, 238 },   { (String)&_iumlStruct, 239 },
	{ (String)&_ethStruct, 240 },     { (String)&_ntildeStruct, 241 },  { (String)&_ograveStruct, 242 },  { (String)&_oacuteStruct, 243 },
	{ (String)&_ocircStruct, 244 },   { (String)&_otildeStruct, 245 },  { (String)&_oumlStruct, 246 },    { (String)&_divideStruct, 247 },
	{ (String)&_oslashStruct, 248 },  { (String)&_ugraveStruct, 249 },  { (String)&_uacuteStruct, 250 },  { (String)&_ucircStruct, 251 },
	{ (String)&_uumlStruct, 252 },    { (String)&_yacuteStruct, 253 },  { (String)&_thornStruct, 254 },   { (String)&_yumlStruct, 255 },

	{ (String)&_OEligStruct, 338 },   { (String)&_oeligStruct, 339 },   { (String)&_ScaronStruct, 352 },  { (String)&_scaronStruct, 353 },
	{ (String)&_YumlStruct, 376 },    { (String)&_fnofStruct, 402 },    { (String)&_circStruct, 710 },    { (String)&_tildeStruct, 732 },

	{ (String)&_AlphaStruct, 913 },   { (String)&_BetaStruct, 914 },    { (String)&_GammaStruct, 915 },   { (String)&_DeltaStruct, 916 },
	{ (String)&_EpsilonStruct, 917 }, { (String)&_ZetaStruct, 918 },    { (String)&_EtaStruct, 919 },     { (String)&_ThetaStruct, 920 },
	{ (String)&_IotaStruct, 921 },    { (String)&_KappaStruct, 922 },   { (String)&_LambdaStruct, 923 },  { (String)&_MuStruct, 924 },
	{ (String)&_NuStruct, 925 },      { (String)&_XiStruct, 926 },      { (String)&_OmicronStruct, 927 }, { (String)&_PiStruct, 928 },
	{ (String)&_RhoStruct, 929 },     { (String)&_SigmaStruct, 931 },   { (String)&_TauStruct, 932 },     { (String)&_UpsilonStruct, 933 },
	{ (String)&_PhiStruct, 934 },     { (String)&_ChiStruct, 935 },     { (String)&_PsiStruct, 936 },     { (String)&_OmegaStruct, 937 },
			
	{ (String)&_alphaStruct, 945 },   { (String)&_betaStruct, 946 },    { (String)&_gammaStruct, 947 },   { (String)&_deltaStruct, 948 },
	{ (String)&_epsilonStruct, 949 }, { (String)&_zetaStruct, 950 },    { (String)&_etaStruct, 951 },     { (String)&_thetaStruct, 952 },
	{ (String)&_iotaStruct, 953 },    { (String)&_kappaStruct, 954 },   { (String)&_lambdaStruct, 955 },  { (String)&_muStruct, 956 },
	{ (String)&_nuStruct, 957 },      { (String)&_xiStruct, 958 },      { (String)&_omicronStruct, 959 }, { (String)&_piStruct, 960 },
	{ (String)&_rhoStruct, 961 },     { (String)&_sigmafStruct, 962 },  { (String)&_sigmaStruct, 963 },   { (String)&_tauStruct, 964 },
	{ (String)&_upsilonStruct, 965 }, { (String)&_phiStruct, 966 },     { (String)&_chiStruct, 967 },     { (String)&_psiStruct, 968 },
	{ (String)&_omegaStruct, 969 },   { (String)&_thetasymStruct, 977 },{ (String)&_upsihStruct, 978 },   { (String)&_pivStruct, 982 },

	{ (String)&_enspStruct, 8194 },   { (String)&_emspStruct, 8195 },   { (String)&_thinspStruct, 8201 }, { (String)&_zwnjStruct, 8204 },
	{ (String)&_zwjStruct, 8205 },    { (String)&_lrmStruct, 8206 },    { (String)&_rlmStruct, 8207 },    { (String)&_ndashStruct, 8211 },
	{ (String)&_mdashStruct, 8212 },  { (String)&_lsquoStruct, 8216 },  { (String)&_rsquoStruct, 8217 },  { (String)&_sbquoStruct, 8218 },
	{ (String)&_ldquoStruct, 8220 },  { (String)&_rdquoStruct, 8221 },  { (String)&_bdquoStruct, 8222 },  { (String)&_daggerStruct, 8224 },
	{ (String)&_DaggerStruct, 8225 }, { (String)&_bullStruct, 8226 },   { (String)&_hellipStruct, 8230 }, { (String)&_permilStruct, 8240 },
	{ (String)&_primeStruct, 8242 },  { (String)&_PrimeStruct, 8243 },  { (String)&_lsaquoStruct, 8249 }, { (String)&_rsaquoStruct, 8250 },
	{ (String)&_olineStruct, 8254 },  { (String)&_fraslStruct, 8260 },  { (String)&_euroStruct, 8364 },   { (String)&_imageStruct, 8465 },
	{ (String)&_weierpStruct, 8472 }, { (String)&_realStruct, 8476 },   { (String)&_tradeStruct, 8482 },  { (String)&_alefsymStruct, 8501 },
	{ (String)&_larrStruct, 8592 },   { (String)&_uarrStruct, 8593 },   { (String)&_rarrStruct, 8594 },   { (String)&_darrStruct, 8595 },
	{ (String)&_harrStruct, 8596 },   { (String)&_crarrStruct, 8629 },  { (String)&_lArrStruct, 8656 },   { (String)&_uArrStruct, 8657 },
	{ (String)&_rArrStruct, 8658 },   { (String)&_dArrStruct, 8659 },   { (String)&_hArrStruct, 8660 },   { (String)&_forallStruct, 8704 },
	{ (String)&_partStruct, 8706 },   { (String)&_existStruct, 8707 },  { (String)&_emptyStruct, 8709 },  { (String)&_nablaStruct, 8711 },
	{ (String)&_isinStruct, 8712 },   { (String)&_notinStruct, 8713 },  { (String)&_niStruct, 8715 },     { (String)&_prodStruct, 8719 },
	{ (String)&_sumStruct, 8721 },    { (String)&_minusStruct, 8722 },  { (String)&_lowastStruct, 8727 }, { (String)&_radicStruct, 8730 },
	{ (String)&_propStruct, 8733 },   { (String)&_infinStruct, 8734 },  { (String)&_angStruct, 8736 },    { (String)&_andStruct, 8743 },
	{ (String)&_orStruct, 8744 },     { (String)&_capStruct, 8745 },    { (String)&_cupStruct, 8746 },    { (String)&_intStruct, 8747 },
	{ (String)&_there4Struct, 8756 }, { (String)&_simStruct, 8764 },    { (String)&_congStruct, 8773 },   { (String)&_asympStruct, 8776 },
	{ (String)&_neStruct, 8800 },     { (String)&_equivStruct, 8801 },  { (String)&_leStruct, 8804 },     { (String)&_geStruct, 8805 },
	{ (String)&_subStruct, 8834 },    { (String)&_supStruct, 8835 },    { (String)&_nsubStruct, 8836 },   { (String)&_subeStruct, 8838 },
	{ (String)&_supeStruct, 8839 },   { (String)&_oplusStruct, 8853 },  { (String)&_otimesStruct, 8855 }, { (String)&_perpStruct, 8869 },
	{ (String)&_sdotStruct, 8901 },   { (String)&_vellipStruct, 8942 }, { (String)&_lceilStruct, 8968 },  { (String)&_rceilStruct, 8969 },
	{ (String)&_lfloorStruct, 8970 }, { (String)&_rfloorStruct, 8971 }, { (String)&_langStruct, 9001 },   { (String)&_rangStruct, 9002 },
	{ (String)&_lozStruct, 9674 },    { (String)&_spadesStruct, 9824 }, { (String)&_clubsStruct, 9827 },  { (String)&_heartsStruct, 9829 },
	{ (String)&_diamsStruct, 9830 },
};

const Int HtmlEntityTableLength = sizeof(HtmlEntityTable) / sizeof(HtmlEntity);
