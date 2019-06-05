using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

using UnicodeSetup.Unicode;

namespace UnicodeSetup
{
	public class Program
	{
		static void Main(string[] args)
		{
			List<string> unicodeData = File.ReadAllLines("Data\\UnicodeData.txt").ToList();

			List<CharacterInfo> parsedUnicodeData = unicodeData
				.Select(line => CleanupLineAndStripComments(line))
				.Where(line => !string.IsNullOrEmpty(line))
				.Select(line => CharacterInfo.Parse(line))
				.ToList();
			Dictionary<uint, CharacterInfo> unicodeLookup = parsedUnicodeData.ToDictionary(c => c.CodeValue);

			List<string> caseFolding = File.ReadAllLines("Data\\CaseFolding.Txt").ToList();

			List<CaseFoldingInfo> parsedCaseFolding = caseFolding
				.Select(line => CleanupLineAndStripComments(line))
				.Where(line => !string.IsNullOrEmpty(line))
				.Select(line => CaseFoldingInfo.Parse(line))
				.ToList();
			Dictionary<uint, CaseFoldingInfo> caseFoldingLookup = parsedCaseFolding
				.Where(c => c.Kind == CaseFoldingKind.Common || c.Kind == CaseFoldingKind.Full)
				.ToDictionary(c => c.CodeValue);

			List<string> specialCasing = File.ReadAllLines("Data\\SpecialCasing.Txt").ToList();

			List<SpecialCasingInfo> parsedSpecialCasing = specialCasing
				.Select(line => CleanupLineAndStripComments(line))
				.Where(line => !string.IsNullOrEmpty(line))
				.Select(line => SpecialCasingInfo.Parse(line))
				.ToList();
			Dictionary<uint, SpecialCasingInfo> specialCasingLookup = parsedSpecialCasing
				.Where(c => string.IsNullOrEmpty(c.Condition))
				.ToDictionary(c => c.CodeValue);

			WriteEnums();

			GenerateCaseConversionTables(unicodeLookup, specialCasingLookup);

			GenerateCaseFoldingTables(caseFoldingLookup);

			GenerateGeneralCategoryTable(unicodeLookup);

			GenerateCanonicalCombiningClassTable(unicodeLookup);

			GenerateDecompositionTable(unicodeLookup);

			GenerateCompositionTable(unicodeLookup);

			GenerateCodePageTables(new Dictionary<string, string>
			{
				{ "cp437.txt", "Cp437" },
				{ "cp1250.txt", "Windows1250" },
				{ "cp1251.txt", "Windows1251" },
				{ "cp1252.txt", "Windows1252" },
				{ "cp1253.txt", "Windows1253" },
				{ "cp1254.txt", "Windows1254" },
				{ "cp1255.txt", "Windows1255" },
				{ "cp1256.txt", "Windows1256" },
				{ "cp1257.txt", "Windows1257" },
				{ "cp1258.txt", "Windows1258" },
				{ "iso-8859-2.txt", "Iso_8859_2" },
				{ "iso-8859-3.txt", "Iso_8859_3" },
				{ "iso-8859-4.txt", "Iso_8859_4" },
				{ "iso-8859-5.txt", "Iso_8859_5" },
				{ "iso-8859-6.txt", "Iso_8859_6" },
				{ "iso-8859-7.txt", "Iso_8859_7" },
				{ "iso-8859-8.txt", "Iso_8859_8" },
				{ "iso-8859-9.txt", "Iso_8859_9" },
				{ "iso-8859-10.txt", "Iso_8859_10" },
				{ "iso-8859-11.txt", "Iso_8859_11" },
				{ "iso-8859-13.txt", "Iso_8859_13" },
				{ "iso-8859-14.txt", "Iso_8859_14" },
				{ "iso-8859-15.txt", "Iso_8859_15" },
				{ "iso-8859-16.txt", "Iso_8859_16" },
			});
		}

		#region Identity Tables

		private static void WriteIdentityTable(string name, StringBuilder output)
		{
			List<string> codePage = new List<string>();
			for (uint i = 0; i < 256; i++)
			{
				codePage.Add("0");
			}

			WriteCodePage(output, "static const Int32", name, codePage, 16);
		}

		#endregion

		#region Case Conversion and Folding

		private static void GenerateCaseConversionTables(Dictionary<uint, CharacterInfo> unicodeLookup, Dictionary<uint, SpecialCasingInfo> specialCasingLookup)
		{
			HashSet<uint> allCasedCharacters = new HashSet<uint>();
			allCasedCharacters.UnionWith(unicodeLookup.Values
				.Where(c => c.UppercaseMapping.HasValue || c.LowercaseMapping.HasValue || c.TitlecaseMapping.HasValue)
				.Select(c => c.CodeValue));
			allCasedCharacters.UnionWith(specialCasingLookup.Keys);

			List<SpecialCasingInfo> casedCharacters = allCasedCharacters
				.Select(v =>
				{
					if (specialCasingLookup.ContainsKey(v)) return specialCasingLookup[v];

					CharacterInfo c = unicodeLookup[v];

					SpecialCasingInfo casingInfo = new SpecialCasingInfo
					{
						CodeValue = v,
						Lowercase = new List<uint> { c.LowercaseMapping.HasValue ? c.LowercaseMapping.Value : c.CodeValue },
						Titlecase = new List<uint> { c.TitlecaseMapping.HasValue ? c.TitlecaseMapping.Value : c.CodeValue },
						Uppercase = new List<uint> { c.UppercaseMapping.HasValue ? c.UppercaseMapping.Value : c.CodeValue },
						Condition = string.Empty,
					};

					return casingInfo;
				})
				.OrderBy(c => c.CodeValue)
				.ToList();

			Dictionary<uint, SpecialCasingInfo> casedLookup = casedCharacters.ToDictionary(c => c.CodeValue);

			List<IGrouping<uint, SpecialCasingInfo>> casedCodePages = casedCharacters.GroupBy(c => (uint)(c.CodeValue & ~0xFF)).ToList();

			WriteRelativeConversionTablesToFile("lowercase.c", "LowercaseTable", "_lowercaseTable", "_l0", casedLookup, casedCodePages,
				code => casedLookup.ContainsKey(code) ? casedLookup[code].Lowercase : null);

			WriteRelativeConversionTablesToFile("uppercase.c", "UppercaseTable", "_uppercaseTable", "_u0", casedLookup, casedCodePages,
				code => casedLookup.ContainsKey(code) ? casedLookup[code].Uppercase : null);

			WriteRelativeConversionTablesToFile("titlecase.c", "TitlecaseTable", "_titlecaseTable", "_t0", casedLookup, casedCodePages,
				code => casedLookup.ContainsKey(code) ? casedLookup[code].Titlecase : null);
		}

		private static void GenerateCaseFoldingTables(Dictionary<uint, CaseFoldingInfo> caseFoldingLookup)
		{
			List<IGrouping<uint, CaseFoldingInfo>> casedCodePages = caseFoldingLookup.Values.GroupBy(c => (uint)(c.CodeValue & ~0xFF)).ToList();
			Func<uint, List<uint>> mapper = code => caseFoldingLookup.ContainsKey(code) ? caseFoldingLookup[code].FoldedValues : null;
			WriteRelativeConversionTablesToFile("casefolding.c", "CaseFoldingTable", "_caseFoldingTable", "_c0", casedCodePages.ToDictionary(grouping => grouping.Key), casedCodePages, mapper);
		}

		#endregion

		#region Shared code-page table generation routines

		private static void WriteRelativeConversionTablesToFile<T, S>(string filename, string identifierName, string subtableName, string identityTableName,
			Dictionary<uint, S> casedLookup, List<IGrouping<uint, T>> casedCodePages, Func<uint, List<uint>> mapper)
		{
			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			output.AppendFormat("#ifdef _MSC_VER\r\n"
				+ "\textern const Int32 {0}ExtendedValues[];\r\n"
				+ "#else\r\n"
				+ "\tstatic const Int32 {0}ExtendedValues[];\r\n"
				+ "#endif\r\n"
				+ "\r\n", subtableName);

			WriteIdentityTable(identityTableName, output);

			List<string> extendedValues = new List<string>();

			foreach (IGrouping<uint, T> page in casedCodePages)
			{
				GenerateRelativeCodePage(output, subtableName, page.Key, extendedValues, mapper);
			}

			GenerateRelativeCodePageIndex(output, identifierName, subtableName, identityTableName,
				casedCodePages.Select(page => page.Key).ToList(), extendedValues, mapper);


			EndOutput(output);

			File.WriteAllText(@"Output\" + filename, output.ToString(), Encoding.UTF8);
		}

		private static void GenerateRelativeCodePageIndex(StringBuilder output, string identifierName, string subtableName, string identityName,
			List<uint> codePageStarts, List<string> extendedValues, Func<uint, List<uint>> mapper)
		{
			WriteCodePage(output, "static const Int32", subtableName + "ExtendedValues", extendedValues, 16);

			List<string> codePage = new List<string>();

			int lastPage = (int)codePageStarts
				.Where(start => NeedRelativeCodePage(start, mapper) != CodePageKind.None)
				.Max(start => start >> 8)
				+ 1;
			for (uint i = 0; i < lastPage; i++)
			{
				codePage.Add(NeedRelativeCodePage(i << 8, mapper) != CodePageKind.None
					? subtableName + i.ToString("X2")
					: identityName);
			}

			WriteCodePage(output, "const Int32 *", "UnicodeTables_" + identifierName, codePage, 8);

			codePage = new List<string>();

			lastPage = (int)codePageStarts
				.Where(start => NeedRelativeCodePage(start, mapper) == CodePageKind.Extended)
				.Max(start => start >> 8)
				+ 1;
			for (uint i = 0; i < lastPage; i++)
			{
				codePage.Add(NeedRelativeCodePage(i << 8, mapper) == CodePageKind.Extended
					? subtableName + "Extended" + i.ToString("X2")
					: "NULL");
			}

			WriteCodePage(output, "const Int32 **", "UnicodeTables_" + identifierName + "Extended", codePage, 8);

			output.AppendFormat("const Int32 UnicodeTables_" + identifierName + "Count = {0};\r\n", lastPage);
		}

		private static void GenerateRelativeCodePage(StringBuilder output, string identifierName,
			uint baseOffset, List<string> extendedValues, Func<uint, List<uint>> mapper)
		{
			CodePageKind codePageKind = NeedRelativeCodePage(baseOffset, mapper);
			if (codePageKind == CodePageKind.None) return;

			List<string> codePage = new List<string>();

			for (uint i = 0; i < 256; i++)
			{
				uint codeValue = i + baseOffset;
				int outputValue;
				List<uint> codeValues = mapper(codeValue);
				if (codeValues != null)
				{
					outputValue = (codeValues.Count > 1) ? -(int)codeValue : (int)codeValues[0] - (int)(i + baseOffset);
				}
				else
				{
					outputValue = 0;
				}
				codePage.Add(outputValue.ToString());
			}

			WriteCodePage(output, "static const Int32", identifierName + (baseOffset >> 8).ToString("X2"), codePage, 16);

			if (codePageKind == CodePageKind.Extended)
			{
				codePage = new List<string>();

				uint end = 0;
				for (uint i = 0; i < 256; i++)
				{
					uint codeValue = i + baseOffset;
					List<uint> codeValues = mapper(codeValue);
					if (codeValue == 0 || (codeValues != null && codeValues.Count > 1))
					{
						if (i + 1 > end) end = i + 1;
					}
				}

				for (uint i = 0; i < end; i++)
				{
					uint codeValue = i + baseOffset;
					List<uint> codeValues = mapper(codeValue);
					if (codeValue == 0)
					{
						int offset = extendedValues.Count;
						codePage.Add(identifierName + "ExtendedValues+" + offset);
						extendedValues.Add("1");
						extendedValues.Add("0");
					}
					else if (codeValues != null)
					{
						if (codeValues.Count > 1)
						{
							int offset = extendedValues.Count;
							codePage.Add(identifierName + "ExtendedValues+" + offset);
							extendedValues.Add(codeValues.Count.ToString());
							extendedValues.AddRange(codeValues.Select(cv => cv.ToString()));
						}
						else
						{
							codePage.Add("NULL");
						}
					}
					else
					{
						codePage.Add("NULL");
					}
				}

				WriteCodePage(output, "static const Int32 *", identifierName + "Extended" + (baseOffset >> 8).ToString("X2"), codePage, 8);
			}
		}

		[Flags]
		private enum CodePageKind
		{
			None = 0,
			SingleValue = 1,
			Extended = 3,
		}

		private static CodePageKind NeedRelativeCodePage(uint baseOffset, Func<uint, List<uint>> mapper)
		{
			if (baseOffset == 0) return CodePageKind.Extended;

			CodePageKind codePageKind = CodePageKind.None;

			for (uint i = 0; i < 256; i++)
			{
				uint codeValue = i + baseOffset;
				List<uint> codes = mapper(codeValue);
				if (codes != null && codes.Count > 0)
				{
					if (codes.Count == 1 && codes[0] != codeValue)
					{
						codePageKind |= CodePageKind.SingleValue;
					}
					if (codes.Count > 1)
					{
						codePageKind |= CodePageKind.Extended;
					}
				}
			}

			return codePageKind;
		}

		#endregion

		#region Enum definitions

		private static void WriteEnums()
		{
			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			output.Append(
@"		public enum GeneralCategory : byte
		{
			// 0..7 range: Normative
			// 8..F range: Informative

			OtherFamily = 0x00,
			OtherNotAssigned = 0x00,
			OtherControl = 0x01,
			OtherFormat = 0x02,
			OtherSurrogate = 0x03,
			OtherPrivateUse = 0x04,

			LetterFamily = 0x10,
			LetterUppercase = 0x11,
			LetterLowercase = 0x12,
			LetterTitlecase = 0x13,
			LetterModifier = 0x19,
			LetterOther = 0x1A,

			MarkFamily = 0x20,
			MarkNonSpacing = 0x21,
			MarkSpacingCombining = 0x22,
			MarkEnclosing = 0x23,

			NumberFamily = 0x30,
			NumberDecimalDigit = 0x31,
			NumberLetter = 0x32,
			NumberOther = 0x33,

			SeparatorFamily = 0x40,
			SeparatorSpace = 0x41,
			SeparatorLine = 0x42,
			SeparatorParagraph = 0x43,

			PunctuationFamily = 0x50,
			PunctuationConnector = 0x59,
			PunctuationDash = 0x5A,
			PunctuationOpen = 0x5B,
			PunctuationClose = 0x5C,
			PunctuationInitialQuote = 0x5D,
			PunctuationFinalQuote = 0x5E,
			PunctuationOther = 0x5F,

			SymbolFamily = 0x60,
			SymbolMath = 0x69,
			SymbolCurrency = 0x6A,
			SymbolModifier = 0x6B,
			SymbolOther = 0x6C,
		}

		public enum CanonicalCombiningClass : byte
		{
			Spacing = 0,
			OverlaysAndInterior = 1,
			Nuktas = 7,
			HiraganaKatakanaVoicingMark = 8,
			Viramas = 9,
			StartOfFixedPositionClasses = 10,
			EndOfFixedPositionClasses = 199,
			BelowLeftAttached = 200,
			BelowAttached = 202,
			BelowRightAttached = 204,
			LeftAttached = 208,
			RightAttached = 210,
			AboveLeftAttached = 212,
			AboveAttached = 214,
			AboveRightAttached = 216,
			BelowLeft = 218,
			Below = 220,
			BelowRight = 222,
			Left = 224,
			Right = 226,
			AboveLeft = 228,
			Above = 230,
			AboveRight = 232,
			DoubleBelow = 232,
			DoubleAbove = 234,
			BelowIotaSubscript = 240,
		}
");

			EndOutput(output);

			File.WriteAllText(@"Output\UnicodeEnums.cpp", output.ToString(), Encoding.UTF8);
		}

		#endregion

		#region General Category Table

		private static readonly CharacterInfo _unusedCharacter = new CharacterInfo
		{
			BidirectionalCategory = BidirectionalCategory.None,
			GeneralCategory = GeneralCategory.Cn,
			CanonicalCombiningClass = 0,
			CharacterDecompositionMappingTag = CharacterDecompositionMappingTag.None,
		};

		private static void GenerateGeneralCategoryTable(Dictionary<uint, CharacterInfo> unicodeLookup)
		{
			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			Dictionary<CodePage, uint> codePages = new Dictionary<CodePage, uint>();
			Dictionary<uint, CodePage> codePageLookup = new Dictionary<uint, CodePage>();
			CodePage currentCodePage = null;
			const int BitsPerPage = 8;
			const int PageMask = (1 << BitsPerPage) - 1;
			for (uint i = 0; i < 0x10000; i++)
			{
				if ((i & PageMask) == 0)
				{
					currentCodePage = new CodePage();
				}
				CharacterInfo charInfo;
				if (!unicodeLookup.TryGetValue(i, out charInfo))
				{
					charInfo = _unusedCharacter;
				}
				currentCodePage.Add(((int)charInfo.GeneralCategory).ToString());
				if ((i & PageMask) == PageMask)
				{
					uint lookupIndex;
					if (codePages.ContainsKey(currentCodePage))
					{
						lookupIndex = codePages[currentCodePage];
						currentCodePage = codePageLookup[lookupIndex];
					}
					else
					{
						codePages.Add(currentCodePage, lookupIndex = i - PageMask);
						currentCodePage.ExternalIdentifier = lookupIndex;
					}
					codePageLookup.Add(i - PageMask, currentCodePage);
				}
			}

			List<string> lookupTable = codePageLookup
				.OrderBy(pair => pair.Key)
				.Select(pair => "_general_" + pair.Value.ExternalIdentifier.ToString("X4"))
				.ToList();

			output.AppendFormat("\t\tconst int GeneralLookupCount = {0};\r\n", lookupTable.Count);
			output.AppendFormat("\t\tconst int GeneralTableCount = {0};\r\n", codePages.Count);
			output.AppendFormat("\t\tconst int GeneralByteCount = {0};\r\n\r\n", codePages.Count * (1 << BitsPerPage));

			foreach (CodePage codePage in codePages.Keys)
			{
				WriteCodePage(output, "static byte", "_general_" + codePage.ExternalIdentifier.ToString("X4"), codePage, 16);
			}

			WriteCodePage(output, "static byte *", "GeneralCategoryTable", lookupTable, 8);

			EndOutput(output);

			File.WriteAllText(@"Output\UnicodeGeneralCategoryTable.cpp", output.ToString(), Encoding.UTF8);
		}

		#endregion

		#region Canonical Combining Class Tables

		private static void GenerateCanonicalCombiningClassTable(Dictionary<uint, CharacterInfo> unicodeLookup)
		{
			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			Dictionary<CodePage, uint> codePages = new Dictionary<CodePage, uint>();
			Dictionary<uint, CodePage> codePageLookup = new Dictionary<uint, CodePage>();
			CodePage currentCodePage = null;
			const int BitsPerPage = 8;
			const int PageMask = (1 << BitsPerPage) - 1;
			for (uint i = 0; i < 0x10000; i++)
			{
				if ((i & PageMask) == 0)
				{
					currentCodePage = new CodePage();
				}
				CharacterInfo charInfo;
				if (!unicodeLookup.TryGetValue(i, out charInfo))
				{
					charInfo = _unusedCharacter;
				}
				currentCodePage.Add(charInfo.CanonicalCombiningClass.ToString());
				if ((i & PageMask) == PageMask)
				{
					uint lookupIndex;
					if (codePages.ContainsKey(currentCodePage))
					{
						lookupIndex = codePages[currentCodePage];
						currentCodePage = codePageLookup[lookupIndex];
					}
					else
					{
						codePages.Add(currentCodePage, lookupIndex = i - PageMask);
						currentCodePage.ExternalIdentifier = lookupIndex;
					}
					codePageLookup.Add(i - PageMask, currentCodePage);
				}
			}

			List<string> lookupTable = codePageLookup
				.OrderBy(pair => pair.Key)
				.Select(pair => "_combining_" + pair.Value.ExternalIdentifier.ToString("X4"))
				.ToList();

			output.AppendFormat("\t\tconst int CombiningLookupCount = {0};\r\n", lookupTable.Count);
			output.AppendFormat("\t\tconst int CombiningTableCount = {0};\r\n", codePages.Count);
			output.AppendFormat("\t\tconst int CombiningByteCount = {0};\r\n\r\n", codePages.Count * (1 << BitsPerPage));

			foreach (CodePage codePage in codePages.Keys)
			{
				WriteCodePage(output, "static byte", "_combining_" + codePage.ExternalIdentifier.ToString("X4"), codePage, 16);
			}

			WriteCodePage(output, "static byte *", "CanonicalCombiningClassTable", lookupTable, 8);

			EndOutput(output);

			File.WriteAllText(@"Output\UnicodeCanonicalCombiningClassTable.cpp", output.ToString(), Encoding.UTF8);
		}


		#endregion

		#region Decomposition Tables

		private static void GenerateDecompositionTable(Dictionary<uint, CharacterInfo> unicodeLookup)
		{
			IEnumerable<CharacterInfo> decomposableCharacters = unicodeLookup.Values
				.Where(info =>
					   info.CharacterDecompositionMappingTag == CharacterDecompositionMappingTag.None
					&& info.CharacterDecompositionMapping != null
					&& info.CharacterDecompositionMapping.Any())
				.ToList();

			IEnumerable<CharacterInfo> decomposedCharacters = decomposableCharacters.Select(ch =>
				new CharacterInfo
				{
					CodeValue = ch.CodeValue,
					CharacterDecompositionMapping = SortDecomposition(RecursivelyDecompose(ch, unicodeLookup), unicodeLookup),
					CharacterDecompositionMappingTag = ch.CharacterDecompositionMappingTag,
				}).ToList();

			Dictionary<uint, CharacterInfo> decompositionLookup = decomposedCharacters.ToDictionary(ch => ch.CodeValue);

			List<IGrouping<uint, CharacterInfo>> codePages = decomposedCharacters.GroupBy(c => (uint)(c.CodeValue & ~0xFF)).ToList();

			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			WriteIdentityTable("_d0", output);

			for (uint i = 0; i < 256; i++)
			{
				GenerateDecompositionCodePage(output, "_decompositionTable", i << 8, decompositionLookup);
			}

			GenerateDecompositionCodePageIndex(output, "DecompositionTable", "_decompositionTable", codePages, decompositionLookup);

			EndOutput(output);

			File.WriteAllText(@"Output\UnicodeDecompositionTable.cpp", output.ToString());
		}

		private static List<uint> RecursivelyDecompose(CharacterInfo characterInfo, Dictionary<uint, CharacterInfo> unicodeLookup)
		{
			List<uint> decomposition = new List<uint>();

			foreach (uint ch in characterInfo.CharacterDecompositionMapping)
			{
				CharacterInfo childInfo;
				if (unicodeLookup.TryGetValue(ch, out childInfo)
					&& childInfo.CharacterDecompositionMappingTag == CharacterDecompositionMappingTag.None
					&& childInfo.CharacterDecompositionMapping != null
					&& childInfo.CharacterDecompositionMapping.Any())
				{
					List<uint> childDecomposition = RecursivelyDecompose(childInfo, unicodeLookup);
					decomposition.AddRange(childDecomposition);
				}
				else
				{
					decomposition.Add(ch);
				}
			}

			return decomposition;
		}

		private static List<uint> SortDecomposition(List<uint> decomposition, Dictionary<uint, CharacterInfo> unicodeLookup)
		{
			List<uint> result = decomposition.OrderBy(code =>
			{
				CharacterInfo info;
				return unicodeLookup.TryGetValue(code, out info) ? info.CanonicalCombiningClass : 0;
			}).ToList();

			return result;
		}

		private static void GenerateDecompositionCodePageIndex(StringBuilder output, string identifierName, string subtableName,
			List<IGrouping<uint, CharacterInfo>> decompositionCodePages, Dictionary<uint, CharacterInfo> decompositionLookup)
		{
			List<string> codePage = new List<string>();

			for (uint i = 0; i < 256; i++)
			{
				codePage.Add(NeedDecompositionCodePage(i << 8, decompositionLookup)
					? subtableName + i.ToString("X2")
					: "_d0");
			}

			WriteCodePage(output, "static int *", identifierName, codePage, 16);

			codePage = new List<string>();

			for (uint i = 0; i < 256; i++)
			{
				codePage.Add(NeedExtendedDecompositionCodePage(i << 8, decompositionLookup)
					? subtableName + "Extended" + i.ToString("X2")
					: "null");
			}

			WriteCodePage(output, "static int **", identifierName + "Extended", codePage, 16);
		}

		private static void GenerateDecompositionCodePage(StringBuilder output, string identifierName,
			uint baseOffset, Dictionary<uint, CharacterInfo> decompositionLookup)
		{
			if (!NeedDecompositionCodePage(baseOffset, decompositionLookup)) return;

			List<string> codePage = new List<string>();

			for (uint i = 0; i < 256; i++)
			{
				uint codeValue = i + baseOffset;
				int outputValue;
				if (decompositionLookup.ContainsKey(codeValue))
				{
					List<uint> codeValues = decompositionLookup[codeValue].CharacterDecompositionMapping;
					outputValue = (codeValues.Count > 1) ? -(int)codeValue : (int)codeValues[0] - (int)codeValue;
				}
				else
				{
					outputValue = 0;
				}
				codePage.Add(outputValue.ToString());
			}

			WriteCodePage(output, "static int", identifierName + (baseOffset >> 8).ToString("X2"), codePage, 16);

			if (NeedExtendedDecompositionCodePage(baseOffset, decompositionLookup))
			{
				codePage = new List<string>();

				for (uint i = 0; i < 256; i++)
				{
					uint codeValue = i + baseOffset;
					if (decompositionLookup.ContainsKey(codeValue))
					{
						List<uint> codeValues = decompositionLookup[codeValue].CharacterDecompositionMapping;
						if (codeValues.Count > 1)
						{
							codePage.Add(StringifyCodeValues(codeValues));
						}
						else
						{
							codePage.Add("NULL");
						}
					}
					else
					{
						codePage.Add("NULL");
					}
				}

				WriteCodePage(output, "static int *", identifierName + "Extended" + (baseOffset >> 8).ToString("X2"), codePage, 8);
			}
		}

		private static bool NeedDecompositionCodePage(uint baseOffset, Dictionary<uint, CharacterInfo> casedLookup)
		{
			for (uint i = 0; i < 256; i++)
			{
				uint codeValue = i + baseOffset;
				if (casedLookup.ContainsKey(codeValue))
				{
					List<uint> codes = casedLookup[codeValue].CharacterDecompositionMapping;
					if (codes != null && codes.Any()) return true;
				}
			}
			return false;
		}

		private static bool NeedExtendedDecompositionCodePage(uint baseOffset, Dictionary<uint, CharacterInfo> casedLookup)
		{
			for (uint i = 0; i < 256; i++)
			{
				uint codeValue = i + baseOffset;
				if (casedLookup.ContainsKey(codeValue))
				{
					List<uint> codes = casedLookup[codeValue].CharacterDecompositionMapping;
					if (codes != null && codes.Count > 1) return true;
				}
			}
			return false;
		}

		#endregion

		#region Composition Tables

		private class CompositionTrieNode
		{
			public int Value;
			public CharacterInfo CharacterInfo;
			public Dictionary<int, CompositionTrieNode> Subtree;
		}

		private class CompositionTrie
		{
			private readonly Dictionary<int, CompositionTrieNode> _rootTree = new Dictionary<int,CompositionTrieNode>();

			public void Insert(CharacterInfo characterInfo)
			{
				Dictionary<int, CompositionTrieNode> tree = _rootTree;

				for (int i = 0; i < characterInfo.CharacterDecompositionMapping.Count; i++)
				{
					uint code = characterInfo.CharacterDecompositionMapping[i];
					bool isLast = (i == characterInfo.CharacterDecompositionMapping.Count - 1);

					CompositionTrieNode node;
					if (!tree.TryGetValue((int)code, out node))
					{
						tree.Add((int)code, node = new CompositionTrieNode
						{
							Value = (int)code,
						});
					}

					if (isLast)
					{
						node.CharacterInfo = characterInfo;
					}
					else
					{
						if (node.Subtree == null)
						{
							node.Subtree = new Dictionary<int, CompositionTrieNode>();
						}
						tree = node.Subtree;
					}
				}
			}

			public void WriteOutput(StringBuilder stringBuilder)
			{
				WriteOutput(stringBuilder, _rootTree, 0, -1);
			}

			private static readonly string[] indents =
			{
				"",
				"\t",
				"\t\t",
				"\t\t\t",
				"\t\t\t\t",
				"\t\t\t\t\t",
				"\t\t\t\t\t\t",
				"\t\t\t\t\t\t\t",
				"\t\t\t\t\t\t\t\t",
				"\t\t\t\t\t\t\t\t\t",
				"\t\t\t\t\t\t\t\t\t\t",
				"\t\t\t\t\t\t\t\t\t\t\t",
			};

			private void WriteOutput(StringBuilder stringBuilder, Dictionary<int, CompositionTrieNode> tree, int depth, int defaultValue)
			{
				stringBuilder.AppendFormat("{0}switch ({1}) {{\r\n", indents[depth+3], (char)(depth + 'a'));
				foreach (int code in tree.Keys.OrderBy(k => k))
				{
					CompositionTrieNode node = tree[code];
					if (node.Subtree != null)
					{
						stringBuilder.AppendFormat("{0}case {1}:\r\n", indents[depth+3], code);
						int childDefaultValue = node.CharacterInfo != null ? ((int)node.CharacterInfo.CodeValue | ((depth + 1) << 24)) : defaultValue;
						if (node.Subtree.Count == 1)
						{
							KeyValuePair<int, CompositionTrieNode> childPair = node.Subtree.First();
							stringBuilder.AppendFormat("{0}return {1} == {2} ? {3} : {4};\r\n",
								indents[depth + 4], (char)(depth + 1 + 'a'), childPair.Key, ((int)childPair.Value.CharacterInfo.CodeValue | ((depth + 2) << 24)), childDefaultValue);
						}
						else
						{
							WriteOutput(stringBuilder, node.Subtree, depth + 1, childDefaultValue);
						}
					}
					else if (node.CharacterInfo != null && depth > 0)
					{
						stringBuilder.AppendFormat("{0}case {1}: return {2};\r\n", indents[depth+3], code, ((int)node.CharacterInfo.CodeValue | ((depth+1) << 24)));
					}
					else if (defaultValue != -1)
					{
						stringBuilder.AppendFormat("{0}case {1}: return {1};\r\n", indents[depth+3], code, defaultValue);
					}
				}
				stringBuilder.AppendFormat("{0}default: return {1};\r\n"
					+ "{0}}}\r\n", indents[depth+3], defaultValue);
			}
		}

		private static void GenerateCompositionTable(Dictionary<uint, CharacterInfo> unicodeLookup)
		{
			IEnumerable<CharacterInfo> decomposableCharacters = unicodeLookup.Values
				.Where(info =>
					   info.CharacterDecompositionMappingTag == CharacterDecompositionMappingTag.None
					&& info.CharacterDecompositionMapping != null
					&& info.CharacterDecompositionMapping.Any())
				.ToList();

			IEnumerable<CharacterInfo> decomposedCharacters = decomposableCharacters.Select(ch =>
				new CharacterInfo
				{
					CodeValue = ch.CodeValue,
					CharacterDecompositionMapping = SortDecomposition(RecursivelyDecompose(ch, unicodeLookup), unicodeLookup),
					CharacterDecompositionMappingTag = ch.CharacterDecompositionMappingTag,
				}).ToList();

			CompositionTrie trie = new CompositionTrie();

			foreach (CharacterInfo info in decomposedCharacters)
			{
				trie.Insert(info);
			}

			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			output.Append("\t\tint UnicodeTables::Compose(int a, int b, int c, int d)\r\n"
				+ "\t\t{\r\n");
			trie.WriteOutput(output);
			output.Append("\t\t}\r\n");

			EndOutput(output);

			File.WriteAllText(@"Output\UnicodeCompositionTable.cpp", output.ToString());
		}

		#endregion

		#region Code Page Tables

		/// <summary>
		/// Like String.Trim(), only this removes all control codes and whitespace in the range of 0 to 32, inclusive.
		/// </summary>
		private static string BetterTrim(string text)
		{
			char[] result = new char[text.Length];
			char ch;
			int src = 0, dest = 0;

			while (src < text.Length && (ch = text[src]) >= '\x00' && ch <= '\x20')
			{
				src++;
			}

			while (src < text.Length)
			{
				result[dest++] = text[src++];
			}

			while (dest > 0 && (ch = text[dest-1]) >= '\x00' && ch <= '\x20')
			{
				dest--;
			}

			return new string(result, 0, dest);
		}

		private static int[] DecodeCodePageLine(string line)
		{
			int hashIndex = line.IndexOf('#');
			if (hashIndex >= 0)
			{
				line = line.Substring(0, hashIndex);
			}

			line = BetterTrim(line);

			if (string.IsNullOrEmpty(line))
				return null;

			string[] pieces = line.Split('\t');
			int[] values = pieces.Select(n => (int)Convert.ToUInt32(n.Substring(2), 16)).ToArray();
			return values;
		}

		private static void GenerateCodePageTables(Dictionary<string, string> codePages)
		{
			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			List<string> undefined = new List<string>();
			for (int i = 0; i < 256; i++)
			{
				undefined.Add("63");
			}
			WriteCodePage(output, "static byte", "_undefined", undefined, 16);

			foreach (KeyValuePair<string, string> pair in codePages)
			{
				string[] lines = File.ReadAllLines("Data\\" + pair.Key);
				List<int[]> mappings = lines.Select(line => DecodeCodePageLine(line)).Where(values => values != null).ToList();
				GenerateCodePageTable(output, pair.Value, mappings);
			}

			EndOutput(output);

			File.WriteAllText(@"Output\UnicodeCodePages.cpp", output.ToString());
		}

		private static void GenerateCodePageTable(StringBuilder output, string codePageName, IEnumerable<int[]> mappings)
		{
			output.Append("\t\t//-----------------------------------------------------------------------------------------\r\n\r\n");

			int[] forwardLookup = new int[256];
			for (int i = 0; i < 256; i++)
			{
				forwardLookup[i] = 0xFFFD;
			}
			foreach (int[] pair in mappings.Where(m => m.Length > 1))
			{
				forwardLookup[pair[0]] = pair[1];
			}

			WriteCodePage(output, "int", "UnicodeTables::" + codePageName + "ToUnicodeTable", forwardLookup.Select(value => value.ToString()), 16);

			Dictionary<int, IEnumerable<int[]>> groupedMappings = mappings
				.Where(m => m.Length > 1)
				.GroupBy(m => m[1] >> 8)
				.ToDictionary(group => group.Key, group => (IEnumerable<int[]>)group);

			int lastCodePage = groupedMappings.Keys.Max();

			List<string> codePagePointers = new List<string>();
			for (int codePage = 0; codePage <= lastCodePage; codePage++)
			{
				IEnumerable<int[]> codePageMappings;
				if (!groupedMappings.TryGetValue(codePage, out codePageMappings))
				{
					codePagePointers.Add("_undefined");
					continue;
				}

				string partialName = string.Format("_{0}_{1:X2}", codePageName, codePage);
				codePagePointers.Add(partialName);

				byte[] reverseLookup = new byte[256];
				for (int i = 0; i < 256; i++)
				{
					reverseLookup[i] = (byte)'?';
				}
				foreach (int[] pair in codePageMappings)
				{
					reverseLookup[pair[1] & 0xFF] = (byte)pair[0];
				}

				WriteCodePage(output, "static byte", partialName, reverseLookup.Select(value => value.ToString()), 16);
			}

			WriteCodePage(output, "byte *", "UnicodeTables::UnicodeTo" + codePageName + "Table", codePagePointers, 8);
		}

		#endregion

		#region General-purpose output methods

		private static void BeginOutput(StringBuilder output)
		{
			output.Append(
				  "//---------------------------------------------------------------------------------------\r\n"
				+ "//  Smile Programming Language Interpreter\r\n"
				+ "//  Copyright 2004-2019 Sean Werkema\r\n"
				+ "//\r\n"
				+ "//  Licensed under the Apache License, Version 2.0 (the \"License\");\r\n"
				+ "//  you may not use this file except in compliance with the License.\r\n"
				+ "//  You may obtain a copy of the License at\r\n"
				+ "//\r\n"
				+ "//      http://www.apache.org/licenses/LICENSE-2.0\r\n"
				+ "//\r\n"
				+ "//  Unless required by applicable law or agreed to in writing, software\r\n"
				+ "//  distributed under the License is distributed on an \"AS IS\" BASIS,\r\n"
				+ "//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\r\n"
				+ "//  See the License for the specific language governing permissions and\r\n"
				+ "//  limitations under the License.\r\n"
				+ "//---------------------------------------------------------------------------------------\r\n"
				+ "\r\n"
				+ "//--------------------------------------------------------------\r\n"
				+ "// WARNING: This file was generated automatically. Do not edit!\r\n"
				+ "//\r\n"
				+ "// The contents of this file are aggregated from data files\r\n"
				+ "// provided by the Unicode Consortium, www.unicode.org.\r\n"
				+ "//--------------------------------------------------------------\r\n"
				+ "\r\n");

			output.Append("#include <smile/internal/unicode.h>\r\n"
				+ "\r\n");
		}

		private static void EndOutput(StringBuilder output)
		{
		}

		private static void WriteCodePage(StringBuilder output, string type, string name, IEnumerable<string> items, int lineLength)
		{
			if (!type.EndsWith("*"))
				type += " ";

			output.AppendFormat("{0}{1}[] =\r\n"
				+ "{{\r\n", type, name);

			int i = 0;
			foreach (string item in items)
			{
				if ((i % lineLength) == 0)
				{
					output.Append("\t");
				}
				output.Append(item);
				output.Append(",");
				if ((i % lineLength) == (lineLength - 1))
				{
					output.Append("\r\n");
				}
				else
				{
					output.Append(" ");
				}
				i++;
			}

			if (((i-1) % lineLength) != (lineLength - 1))
			{
				output.Append("\r\n");
			}

			output.Append("};\r\n\r\n");
		}

		private static string StringifyCodeValues(List<uint> codeValues)
		{
			StringBuilder output = new StringBuilder();

			output.Append("{");

			bool isFirst = true;
			foreach (uint value in codeValues)
			{
				if (!isFirst)
				{
					output.Append(",");
				}
				output.AppendFormat("{0}", value);
				isFirst = false;
			}

			output.Append("}");

			return output.ToString();
		}

		#endregion

		#region Input helper methods

		private static string CleanupLineAndStripComments(string text)
		{
			int commentIndex = text.IndexOf('#');
			if (commentIndex >= 0)
			{
				text = text.Substring(0, commentIndex);
			}
			return text.Trim();
		}

		#endregion
	}
}
