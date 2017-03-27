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
	{ &_quotStruct, 34 },     { &_ampStruct, 38 },      { &_aposStruct, 39 },
	{ &_ltStruct, 60 },       { &_gtStruct, 62 },
			
	{ &_nbspStruct, 160 },    { &_iexclStruct, 161 },   { &_centStruct, 162 },    { &_poundStruct, 163 },
	{ &_currenStruct, 164 },  { &_yenStruct, 165 },     { &_brvbarStruct, 166 },  { &_sectStruct, 167 },
	{ &_umlStruct, 168 },     { &_copyStruct, 169 },    { &_ordfStruct, 170 },    { &_laquoStruct, 171 },
	{ &_notStruct, 172 },     { &_shyStruct, 173 },     { &_regStruct, 174 },     { &_macrStruct, 175 },
	{ &_degStruct, 176 },     { &_plusmnStruct, 177 },  { &_sup2Struct, 178 },    { &_sup3Struct, 179 },
	{ &_acuteStruct, 180 },   { &_microStruct, 181 },   { &_paraStruct, 182 },    { &_middotStruct, 183 },
	{ &_cedilStruct, 184 },   { &_sup1Struct, 185 },    { &_ordmStruct, 186 },    { &_raquoStruct, 187 },
	{ &_frac14Struct, 188 },  { &_frac12Struct, 189 },  { &_frac34Struct, 190 },  { &_iquestStruct, 191 },

	{ &_AgraveStruct, 192 },  { &_AacuteStruct, 193 },  { &_AcircStruct, 194 },   { &_AtildeStruct, 195 },
	{ &_AumlStruct, 196 },    { &_AringStruct, 197 },   { &_AEligStruct, 198 },   { &_CcedilStruct, 199 },
	{ &_EgraveStruct, 200 },  { &_EacuteStruct, 201 },  { &_EcircStruct, 202 },   { &_EumlStruct, 203 },
	{ &_IgraveStruct, 204 },  { &_IacuteStruct, 205 },  { &_IcircStruct, 206 },   { &_IumlStruct, 207 },
	{ &_ETHStruct, 208 },     { &_NtildeStruct, 209 },  { &_OgraveStruct, 210 },  { &_OacuteStruct, 211 },
	{ &_OcircStruct, 212 },   { &_OtildeStruct, 213 },  { &_OumlStruct, 214 },    { &_timesStruct, 215 },
	{ &_OslashStruct, 216 },  { &_UgraveStruct, 217 },  { &_UacuteStruct, 218 },  { &_UcircStruct, 219 },
	{ &_UumlStruct, 220 },    { &_YacuteStruct, 221 },  { &_THORNStruct, 222 },   { &_szligStruct, 223 },

	{ &_agraveStruct, 224 },  { &_aacuteStruct, 225 },  { &_acircStruct, 226 },   { &_atildeStruct, 227 },
	{ &_aumlStruct, 228 },    { &_aringStruct, 229 },   { &_aeligStruct, 230 },   { &_ccedilStruct, 231 },
	{ &_egraveStruct, 232 },  { &_eacuteStruct, 233 },  { &_ecircStruct, 234 },   { &_eumlStruct, 235 },
	{ &_igraveStruct, 236 },  { &_iacuteStruct, 237 },  { &_icircStruct, 238 },   { &_iumlStruct, 239 },
	{ &_ethStruct, 240 },     { &_ntildeStruct, 241 },  { &_ograveStruct, 242 },  { &_oacuteStruct, 243 },
	{ &_ocircStruct, 244 },   { &_otildeStruct, 245 },  { &_oumlStruct, 246 },    { &_divideStruct, 247 },
	{ &_oslashStruct, 248 },  { &_ugraveStruct, 249 },  { &_uacuteStruct, 250 },  { &_ucircStruct, 251 },
	{ &_uumlStruct, 252 },    { &_yacuteStruct, 253 },  { &_thornStruct, 254 },   { &_yumlStruct, 255 },

	{ &_OEligStruct, 338 },   { &_oeligStruct, 339 },   { &_ScaronStruct, 352 },  { &_scaronStruct, 353 },
	{ &_YumlStruct, 376 },    { &_fnofStruct, 402 },    { &_circStruct, 710 },    { &_tildeStruct, 732 },

	{ &_AlphaStruct, 913 },   { &_BetaStruct, 914 },    { &_GammaStruct, 915 },   { &_DeltaStruct, 916 },
	{ &_EpsilonStruct, 917 }, { &_ZetaStruct, 918 },    { &_EtaStruct, 919 },     { &_ThetaStruct, 920 },
	{ &_IotaStruct, 921 },    { &_KappaStruct, 922 },   { &_LambdaStruct, 923 },  { &_MuStruct, 924 },
	{ &_NuStruct, 925 },      { &_XiStruct, 926 },      { &_OmicronStruct, 927 }, { &_PiStruct, 928 },
	{ &_RhoStruct, 929 },     { &_SigmaStruct, 931 },   { &_TauStruct, 932 },     { &_UpsilonStruct, 933 },
	{ &_PhiStruct, 934 },     { &_ChiStruct, 935 },     { &_PsiStruct, 936 },     { &_OmegaStruct, 937 },
			
	{ &_alphaStruct, 945 },   { &_betaStruct, 946 },    { &_gammaStruct, 947 },   { &_deltaStruct, 948 },
	{ &_epsilonStruct, 949 }, { &_zetaStruct, 950 },    { &_etaStruct, 951 },     { &_thetaStruct, 952 },
	{ &_iotaStruct, 953 },    { &_kappaStruct, 954 },   { &_lambdaStruct, 955 },  { &_muStruct, 956 },
	{ &_nuStruct, 957 },      { &_xiStruct, 958 },      { &_omicronStruct, 959 }, { &_piStruct, 960 },
	{ &_rhoStruct, 961 },     { &_sigmafStruct, 962 },  { &_sigmaStruct, 963 },   { &_tauStruct, 964 },
	{ &_upsilonStruct, 965 }, { &_phiStruct, 966 },     { &_chiStruct, 967 },     { &_psiStruct, 968 },
	{ &_omegaStruct, 969 },   { &_thetasymStruct, 977 },{ &_upsihStruct, 978 },   { &_pivStruct, 982 },

	{ &_enspStruct, 8194 },   { &_emspStruct, 8195 },   { &_thinspStruct, 8201 }, { &_zwnjStruct, 8204 },
	{ &_zwjStruct, 8205 },    { &_lrmStruct, 8206 },    { &_rlmStruct, 8207 },    { &_ndashStruct, 8211 },
	{ &_mdashStruct, 8212 },  { &_lsquoStruct, 8216 },  { &_rsquoStruct, 8217 },  { &_sbquoStruct, 8218 },
	{ &_ldquoStruct, 8220 },  { &_rdquoStruct, 8221 },  { &_bdquoStruct, 8222 },  { &_daggerStruct, 8224 },
	{ &_DaggerStruct, 8225 }, { &_bullStruct, 8226 },   { &_hellipStruct, 8230 }, { &_permilStruct, 8240 },
	{ &_primeStruct, 8242 },  { &_PrimeStruct, 8243 },  { &_lsaquoStruct, 8249 }, { &_rsaquoStruct, 8250 },
	{ &_olineStruct, 8254 },  { &_fraslStruct, 8260 },  { &_euroStruct, 8364 },   { &_imageStruct, 8465 },
	{ &_weierpStruct, 8472 }, { &_realStruct, 8476 },   { &_tradeStruct, 8482 },  { &_alefsymStruct, 8501 },
	{ &_larrStruct, 8592 },   { &_uarrStruct, 8593 },   { &_rarrStruct, 8594 },   { &_darrStruct, 8595 },
	{ &_harrStruct, 8596 },   { &_crarrStruct, 8629 },  { &_lArrStruct, 8656 },   { &_uArrStruct, 8657 },
	{ &_rArrStruct, 8658 },   { &_dArrStruct, 8659 },   { &_hArrStruct, 8660 },   { &_forallStruct, 8704 },
	{ &_partStruct, 8706 },   { &_existStruct, 8707 },  { &_emptyStruct, 8709 },  { &_nablaStruct, 8711 },
	{ &_isinStruct, 8712 },   { &_notinStruct, 8713 },  { &_niStruct, 8715 },     { &_prodStruct, 8719 },
	{ &_sumStruct, 8721 },    { &_minusStruct, 8722 },  { &_lowastStruct, 8727 }, { &_radicStruct, 8730 },
	{ &_propStruct, 8733 },   { &_infinStruct, 8734 },  { &_angStruct, 8736 },    { &_andStruct, 8743 },
	{ &_orStruct, 8744 },     { &_capStruct, 8745 },    { &_cupStruct, 8746 },    { &_intStruct, 8747 },
	{ &_there4Struct, 8756 }, { &_simStruct, 8764 },    { &_congStruct, 8773 },   { &_asympStruct, 8776 },
	{ &_neStruct, 8800 },     { &_equivStruct, 8801 },  { &_leStruct, 8804 },     { &_geStruct, 8805 },
	{ &_subStruct, 8834 },    { &_supStruct, 8835 },    { &_nsubStruct, 8836 },   { &_subeStruct, 8838 },
	{ &_supeStruct, 8839 },   { &_oplusStruct, 8853 },  { &_otimesStruct, 8855 }, { &_perpStruct, 8869 },
	{ &_sdotStruct, 8901 },   { &_vellipStruct, 8942 }, { &_lceilStruct, 8968 },  { &_rceilStruct, 8969 },
	{ &_lfloorStruct, 8970 }, { &_rfloorStruct, 8971 }, { &_langStruct, 9001 },   { &_rangStruct, 9002 },
	{ &_lozStruct, 9674 },    { &_spadesStruct, 9824 }, { &_clubsStruct, 9827 },  { &_heartsStruct, 9829 },
	{ &_diamsStruct, 9830 },
};

const Int HtmlEntityTableLength = sizeof(HtmlEntityTable) / sizeof(HtmlEntity);
