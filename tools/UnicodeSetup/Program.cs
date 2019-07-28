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

			GenerateCaseConversionTables(unicodeLookup, specialCasingLookup);

			GenerateCaseFoldingTables(caseFoldingLookup);

			GenerateGeneralCategoryTable(unicodeLookup);

			GenerateCanonicalCombiningClassTable(unicodeLookup);

			GenerateDecompositionTable(unicodeLookup);

			GenerateCompositionTable(unicodeLookup);

			GenerateCodePageTables("codepages_cpx.c", new Dictionary<string, string>
			{
				{ "cp437.txt", "Cp437" },
			});

			GenerateCodePageTables("codepages_win125x.c", new Dictionary<string, string>
			{
				{ "cp1250.txt", "Windows1250" },
				{ "cp1251.txt", "Windows1251" },
				{ "cp1252.txt", "Windows1252" },
				{ "cp1253.txt", "Windows1253" },
				{ "cp1254.txt", "Windows1254" },
				{ "cp1255.txt", "Windows1255" },
				{ "cp1256.txt", "Windows1256" },
				{ "cp1257.txt", "Windows1257" },
				{ "cp1258.txt", "Windows1258" },
			});

			GenerateCodePageTables("codepages_iso8859.c", new Dictionary<string, string>
			{
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
			}, Iso88591Table);
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
			Dictionary<uint, uint> codePageMap = new Dictionary<uint, uint>();
			Dictionary<uint, uint> paragraphOffsets = new Dictionary<uint, uint>();
			Dictionary<uint, int> usages = new Dictionary<uint, int>();

			const int BitsPerPage = 4;
			const int PageMask = (1 << BitsPerPage) - 1;

			CodePage zeroPage = new CodePage();
			for (uint i = 0; i < (1 << BitsPerPage); i++)
			{
				zeroPage.Add(GeneralCategory.Cn.ToString());
			}
			codePages.Add(zeroPage, 0);

			uint lastCode = unicodeLookup.Keys.Max();
			uint endPoint = (uint)((lastCode + PageMask) & ~PageMask);

			CodePage currentCodePage = null;
			for (uint i = 0; i < endPoint; i++)
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
				currentCodePage.Add(charInfo.GeneralCategory.ToString());
				if ((i & PageMask) == PageMask)
				{
					uint lookupIndex;
					if (!codePages.TryGetValue(currentCodePage, out lookupIndex))
					{
						codePages.Add(currentCodePage, lookupIndex = (uint)codePages.Count);
					}

					if (!usages.ContainsKey(lookupIndex))
						usages.Add(lookupIndex, 1);
					else usages[lookupIndex]++;

					codePageMap.Add((i - PageMask) / (1 << BitsPerPage), lookupIndex);
				}
			}

			output.AppendFormat("#ifdef _MSC_VER\r\n"
				+ "\textern const SByte Unicode_GeneralCategoryData[];\r\n"
				+ "\textern const UInt16 Unicode_GeneralCategoryBmpLookup[];\r\n"
				+ "\textern const Unicode_ExtendedTuple Unicode_GeneralCategoryExtendedLookup[];\r\n"
				+ "\textern const Int Unicode_GeneralCategoryExtendedLookupCount;\r\n"
				+ "#else\r\n"
				+ "\tstatic const SByte Unicode_GeneralCategoryData[];\r\n"
				+ "\tstatic const UInt16 Unicode_GeneralCategoryBmpLookup[];\r\n"
				+ "\tstatic const Unicode_ExtendedTuple Unicode_GeneralCategoryExtendedLookup[];\r\n"
				+ "\tstatic const Int Unicode_GeneralCategoryExtendedLookupCount;\r\n"
				+ "#endif\r\n"
				+ "\r\n");

			foreach (GeneralCategory value in Enum.GetValues(typeof(GeneralCategory)))
			{
				output.AppendFormat("#define {0} 0x{1:X2}\r\n", value.ToString(), (int)value);
			}
			output.Append("\r\n");

			output.AppendFormat("// General Category values for various sets of code points.\r\n"
				+ "// Each row of 16 values is indexed by the lookup tables below.\r\n"
				+ "// Rows near the top are more likely to be used than rows near the bottom.\r\n"
				+ "const SByte Unicode_GeneralCategoryData[{0}] = {{\r\n",
				codePages.Count << BitsPerPage);

			uint offset = 0;
			foreach (KeyValuePair<CodePage, uint> pair in codePages.OrderByDescending(pair => usages[pair.Value]).ThenBy(pair => pair.Value))
			{
				Tuple<string, uint> valuesAndCount = CompressPageValues(pair.Key.ToArray());
				output.Append("\t");
				output.Append(valuesAndCount.Item1);
				output.Append(",\t// Row ");
				output.Append(offset >> BitsPerPage);
				output.Append(", used ");
				output.Append(usages[pair.Value]);
				output.Append(" times\r\n");
				paragraphOffsets[pair.Value] = offset;
				offset += valuesAndCount.Item2;
			}

			output.Append("};\r\n\r\n");

			output.AppendFormat("// The paragraphs (sets of code points) in the Basic Multilingual Plane.\r\n"
				+ "// Each value is an index to a row of the general-category data above.\r\n"
				+ "const UInt16 Unicode_GeneralCategoryBmpLookup[{0}] = {{",
				0x10000 >> BitsPerPage);

			List<uint> mapValues = codePageMap.OrderBy(pair => pair.Key).Select(pair => pair.Value).ToList();
			for (int i = 0; i < (0x10000 >> BitsPerPage); i++)
			{
				if ((i & PageMask) == 0)
					output.Append("\r\n\t");
				output.Append(paragraphOffsets[mapValues[i]] >> BitsPerPage);
				output.Append(",");
			}

			output.Append("\r\n};\r\n\r\n");

			int extendedCount = 0;
			for (int i = (0x10000 >> BitsPerPage); i < mapValues.Count; i++)
			{
				if (mapValues[i] != 0) extendedCount++;
			}

			output.AppendFormat("// All assigned paragraphs above the BMP, sorted for efficient binary search.\r\n"
				+ "// First value is paragraph ID (starting code point = (id + " + (0x10000 >> BitsPerPage) + ") << " + BitsPerPage + ").\r\n"
				+ "// Second value is an index into a row of the general-category data above.\r\n"
				+ "const Unicode_ExtendedTuple Unicode_GeneralCategoryExtendedLookup[{0}] = {{",
				extendedCount);

			for (int i = (0x10000 >> BitsPerPage); i < mapValues.Count; i++)
			{
				if (mapValues[i] != 0)
				{
					output.Append("\r\n\t{ ");
					output.Append(i - (0x10000 >> BitsPerPage));
					output.Append(",");
					output.Append(paragraphOffsets[mapValues[i]] >> BitsPerPage);
					output.Append(" },");
				}
			}

			output.Append("\r\n};\r\n\r\n");

			output.Append("const Int Unicode_GeneralCategoryExtendedLookupCount = " + extendedCount + ";\r\n");

			output.Append(@"
/// <summary>
/// Given a Unicode code point known to be > U+FFFF, this uses an efficient binary search
/// (with embedded fast linear search) to look up its General Category assignment.
/// 
/// Note: While a general binary search runs in O(lg n) time, this implementation will
/// always run in O(1) time because of the limited dataset to test against (but note that
/// O(1) is NOT the same as cryptographically-secure constant time; the execution time will
/// be variable, but it will still be within O(1)).  For current datasets, this implementation
/// will test the code-point against at most ~16 values before finding the correct answer.
/// </summary>
/// <param name=""codePoint"">A Unicode code point that must be higher than U+FFFF.</param>
/// <returns>The General Category assignment for that code point, or 0 if it is an unknown code point (or <= U+FFFF).</returns>
Byte Unicode_GetGeneralCategoryExtended(UInt32 codePoint)
{
	Int paragraphId = (codePoint - 0x10000) >> " + BitsPerPage + @";
	Int start = 0;
	Int end = Unicode_GeneralCategoryExtendedLookupCount;
	const Unicode_ExtendedTuple *tuples;

	// Use binary search until we get to a smallish range.  For the current Unicode dataset,
	// this will loop at most seven times.
	while (start + 8 < end)
	{
		Int midpt = (start + end) >> 1;
		UInt16 midptValue = Unicode_GeneralCategoryExtendedLookup[midpt].paragraphId;
		if (paragraphId == midptValue)
		{
			return Unicode_GeneralCategoryData[Unicode_GeneralCategoryExtendedLookup[midpt].offset + (codePoint & 0xF)];
		}
		else if (paragraphId < midptValue)
		{
			end = midpt;
		}
		else
		{
			start = midpt + 1;
		}
	}

#define TRY_AT(index) \
	if (paragraphId == tuples[index].paragraphId) \
		return Unicode_GeneralCategoryData[tuples[index].offset + (codePoint & 0xF)]

	// Use an unrolled linear search (via computed goto) when the range gets small,
	// since the binary reduction gets very expensive for small ranges.  This will also
	// test about eight values.
	tuples = Unicode_GeneralCategoryExtendedLookup + start;
	switch (end - start) {
		case 8: TRY_AT(7);
		case 7: TRY_AT(6);
		case 6: TRY_AT(5);
		case 5: TRY_AT(4);
		case 4: TRY_AT(3);
		case 3: TRY_AT(2);
		case 2: TRY_AT(1);
		case 1: TRY_AT(0);
		case 0:
		default: return 0;
	}
}

");

			EndOutput(output);

			File.WriteAllText(@"Output\category.c", output.ToString(), Encoding.UTF8);
		}

		private static Tuple<string, uint> CompressPageValues(IList<string> values)
		{
			return new Tuple<string, uint>(string.Join(", ", values), (uint)values.Count);

			/*
			// This compression algorithm seemed like a great idea, but in reality, it only ends up
			// saving about 17% of the size of the data tables.  Adding a lot of runtime complexity
			// and a runtime performance cost just so that we can cut the dataset from 18.5 KiB to
			// 15.3 KiB simply isn't worth it, so this is all commented out, and we just store the
			// data verbatim.  The compression algorithm is in C#, and the C decompression code is
			// included below for historical reference:

			Inline Byte Unicode_SequenceToGeneralCategory(SByte *sequence, UInt32 codePoint)
			{
				SByte runCount = sequence[0];
				UInt32 offset = codePoint & 0xF;

				return runCount == 0 ? (                                    sequence[offset            + 1])
					 : runCount >  0 ? (offset <   runCount ? sequence[1] : sequence[offset - runCount + 2])
					 :                 (offset >= -runCount ? sequence[1] : sequence[offset            + 2]);
			}

			int forwardRun = MeasureRun(values, 0, values.Count, +1);
			int backwardRun = MeasureRun(values, values.Count - 1, -1, -1);

			if (forwardRun == values.Count)
			{
				// If the run covers all of the values, emit a simple pair that covers all of them.
				return new Tuple<string, uint>(string.Format("+{0,2}, {1}", values.Count, values[0]), 2);
			}
			else if (forwardRun > backwardRun && forwardRun > 2)
			{
				// If the forward run is large, emit the starting set compressed, and the rest verbatim.
				string uncompressed = string.Join(", ", values.Skip(forwardRun));
				return new Tuple<string, uint>(string.Format("+{0,2}, {1}, {2}", forwardRun, values[0], uncompressed),
					(uint)(values.Count - forwardRun + 2));
			}
			else if (backwardRun > 2)
			{
				// If the backward run is large, emit the ending set compressed, and the rest verbatim.
				string uncompressed = string.Join(", ", values.Take(values.Count - backwardRun));
				return new Tuple<string, uint>(string.Format("-{0,2}, {1}, {2}", backwardRun, values[values.Count - 1], uncompressed),
					(uint)(values.Count - backwardRun + 2));
			}
			else
			{
				// Not easily compressible, so emit a '0' run, followed by all values verbatim.
				return new Tuple<string, uint>("  0, " + string.Join(", ", values), (uint)values.Count + 1);
			}

			static int MeasureRun(IList<string> values, int start, int end, int step)
			{
				string firstValue = values[start];
				int count = 0;

				for (start += step, count++; start != end && values[start] == firstValue; start += step, count++) ;

				return count;
			}
			*/
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

			output.AppendFormat("const Int UnicodeTables_CanonicalCombiningClassLookupCount = {0};\r\n", lookupTable.Count);
			output.AppendFormat("const Int UnicodeTables_CanonicalCombiningClassTableCount = {0};\r\n", codePages.Count);
			output.AppendFormat("const Int UnicodeTables_CanonicalCombiningClassByteCount = {0};\r\n\r\n", codePages.Count * (1 << BitsPerPage));

			foreach (CodePage codePage in codePages.Keys)
			{
				WriteCodePage(output, "static const Byte", "_combining_" + codePage.ExternalIdentifier.ToString("X4"), codePage, 16);
			}

			WriteCodePage(output, "const Byte *", "UnicodeTables_CanonicalCombiningClassTable", lookupTable, 8);

			EndOutput(output);

			File.WriteAllText(@"Output\combiningclass.c", output.ToString(), Encoding.UTF8);
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

			List<uint> extendedValues = new List<uint>();

			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			output.Append(@"#ifdef _MSC_VER
	extern const Int32 _decompositionTableExtendedValues[];
#else
	static const Int32 _decompositionTableExtendedValues[];
#endif

");

			WriteIdentityTable("_d0", output);

			for (uint i = 0; i < 256; i++)
			{
				GenerateDecompositionCodePage(output, "_decompositionTable", i << 8, decompositionLookup, extendedValues);
			}

			WriteCodePage(output, "static const Int32", "_decompositionTableExtendedValues",
				extendedValues.Select(v => v.ToString()), 16);

			GenerateDecompositionCodePageIndex(output, "UnicodeTables_DecompositionTable", "_decompositionTable", codePages, decompositionLookup);

			EndOutput(output);

			File.WriteAllText(@"Output\decomposition.c", output.ToString());
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

			output.Append("const Int32 " + identifierName + "Count = 256;\r\n\r\n");

			WriteCodePage(output, "const Int32 *", identifierName, codePage, 16);

			codePage = new List<string>();

			for (uint i = 0; i < 256; i++)
			{
				codePage.Add(NeedExtendedDecompositionCodePage(i << 8, decompositionLookup)
					? subtableName + "Extended" + i.ToString("X2")
					: "NULL");
			}

			WriteCodePage(output, "const Int32 **", identifierName + "Extended", codePage, 16);
		}

		private static void GenerateDecompositionCodePage(StringBuilder output, string identifierName,
			uint baseOffset, Dictionary<uint, CharacterInfo> decompositionLookup, List<uint> extendedValues)
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

			WriteCodePage(output, "static const Int32", identifierName + (baseOffset >> 8).ToString("X2"), codePage, 16);

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
							int offset = extendedValues.Count;
							extendedValues.Add((uint)codeValues.Count);
							extendedValues.AddRange(codeValues);
							codePage.Add("_decompositionTableExtendedValues+" + offset);
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

			private static readonly string[] _depthFlags = { "", "ONE_POINT", "TWO_POINTS", "THREE_POINTS", "FOUR_POINTS" };

			private string StringifyCode(uint codeValue, int depth = 0)
			{
				return StringifyCode((int)codeValue, depth);
			}

			private string StringifyCode(int codeValue, int depth = 0)
			{
				if (codeValue == -1)
					return "NO_MATCH";

				if (depth == 0)
					return string.Format("0x{0:X}", codeValue);

				return string.Format("(0x{0:X} | {1})", codeValue, _depthFlags[depth]);
			}

			private void WriteOutput(StringBuilder stringBuilder, Dictionary<int, CompositionTrieNode> tree, int depth, int defaultValue)
			{
				stringBuilder.AppendFormat("{0}switch ({1}) {{\r\n", indents[depth+1], (char)(depth + 'a'));
				foreach (int code in tree.Keys.OrderBy(k => k))
				{
					CompositionTrieNode node = tree[code];
					if (node.Subtree != null)
					{
						stringBuilder.AppendFormat("{0}case {1}:\r\n", indents[depth+1], StringifyCode(code));
						int childDefaultValue = node.CharacterInfo != null ? (int)node.CharacterInfo.CodeValue : defaultValue;
						if (node.Subtree.Count == 1)
						{
							KeyValuePair<int, CompositionTrieNode> childPair = node.Subtree.First();
							stringBuilder.AppendFormat("{0}return {1} == {2} ? {3} : {4};\r\n",
								indents[depth + 2], (char)(depth + 1 + 'a'),
								StringifyCode(childPair.Key),
								StringifyCode(childPair.Value.CharacterInfo.CodeValue, depth + 2),
								StringifyCode(childDefaultValue, depth + 1));
						}
						else if (node.Subtree.Count == 2)
						{
							KeyValuePair<int, CompositionTrieNode> firstPair = node.Subtree.First();
							KeyValuePair<int, CompositionTrieNode> secondPair = node.Subtree.Skip(1).First();
							stringBuilder.AppendFormat("{0}return {1} == {2} ? {3}\r\n"
									+ "{0}\t: {4} == {5} ? {6}\r\n"
									+ "{0}\t: {7};\r\n",
								indents[depth + 2],
								(char)(depth + 1 + 'a'), StringifyCode(firstPair.Key), StringifyCode(firstPair.Value.CharacterInfo.CodeValue, depth + 2),
								(char)(depth + 1 + 'a'), StringifyCode(secondPair.Key), StringifyCode(secondPair.Value.CharacterInfo.CodeValue, depth + 2),
								StringifyCode(childDefaultValue, depth + 1));
						}
						else
						{
							WriteOutput(stringBuilder, node.Subtree, depth + 1, childDefaultValue);
						}
					}
					else if (node.CharacterInfo != null && depth > 0)
					{
						stringBuilder.AppendFormat("{0}case {1}: return {2};\r\n",
							indents[depth+1], StringifyCode(code), StringifyCode(node.CharacterInfo.CodeValue, depth+1));
					}
					else if (defaultValue != -1)
					{
						stringBuilder.AppendFormat("{0}case {1}: return {2};\r\n",
							indents[depth+1], StringifyCode(code), StringifyCode(defaultValue, depth+1));
					}
				}
				stringBuilder.AppendFormat("{0}default: return {1};\r\n"
					+ "{0}}}\r\n", indents[depth+1], StringifyCode(defaultValue, depth));
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

			output.Append(
				  "#define NO_MATCH\t(-1)\r\n"
				+ "\r\n"
				+ "#define ONE_POINT\t(1 << 24)\r\n"
				+ "#define TWO_POINTS\t(2 << 24)\r\n"
				+ "#define THREE_POINTS\t(3 << 24)\r\n"
				+ "#define FOUR_POINTS\t(4 << 24)\r\n"
				+ "\r\n");

			output.Append("Int32 Unicode_Compose(Int32 a, Int32 b, Int32 c, Int32 d)\r\n"
				+ "{\r\n");
			trie.WriteOutput(output);
			output.Append("}\r\n");

			EndOutput(output);

			File.WriteAllText(@"Output\composition.c", output.ToString());
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

		private const string Iso88591Table = @"
//-----------------------------------------------------------------------------------------
// ISO 8859-1 is represented as a special mapping of raw bytes to the low 256 code points.
//
// This is a bit more than the real ISO 8859-1 has defined.  ISO 8859-1 omits a meaningful
// transform for byte values in the range of 0x80-0xBF, so those would normally be
// transformed to Unicode U+FFFD, or 'no character'.  But when defined with equivalent
// values for 0x80-0xBF, this code page has the unique property that it's can safely
// round-trip between raw byte values and Unicode, which is a such a highly-desirable
// property that it's better to define it this way than to use the official definition and
// have a hole in the middle of the table.

const UInt16 UnicodeTables_Iso_8859_1ToUnicodeTable[] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
};

static const Byte _Iso_8859_1_00[] =
{
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
	96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
	224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
};

const Byte *UnicodeTables_UnicodeToIso_8859_1Table[] =
{
	_Iso_8859_1_00,
};

const Int UnicodeTables_UnicodeToIso_8859_1TableCount = sizeof(UnicodeTables_UnicodeToIso_8859_1Table) / sizeof(const Byte **);
";

		private static void GenerateCodePageTables(string outputFile, Dictionary<string, string> codePages, string extra = null)
		{
			StringBuilder output = new StringBuilder();

			BeginOutput(output);

			List<string> undefined = new List<string>();
			for (int i = 0; i < 256; i++)
			{
				undefined.Add("63");
			}
			WriteCodePage(output, "static const Byte", "_undefined", undefined, 16);

			if (!string.IsNullOrEmpty(extra))
			{
				output.Append(extra);
			}

			foreach (KeyValuePair<string, string> pair in codePages)
			{
				string[] lines = File.ReadAllLines("Data\\" + pair.Key);
				List<int[]> mappings = lines.Select(line => DecodeCodePageLine(line)).Where(values => values != null).ToList();
				GenerateCodePageTable(output, pair.Value, mappings);
			}

			EndOutput(output);

			File.WriteAllText(@"Output\" + outputFile, output.ToString());
		}

		private static void GenerateCodePageTable(StringBuilder output, string codePageName, IEnumerable<int[]> mappings)
		{
			output.Append("//-----------------------------------------------------------------------------------------\r\n\r\n");

			int[] forwardLookup = new int[256];
			for (int i = 0; i < 256; i++)
			{
				forwardLookup[i] = 0xFFFD;
			}
			foreach (int[] pair in mappings.Where(m => m.Length > 1))
			{
				forwardLookup[pair[0]] = pair[1];
			}

			WriteCodePage(output, "const UInt16", "UnicodeTables_" + codePageName + "ToUnicodeTable", forwardLookup.Select(value => value.ToString()), 16);

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

				WriteCodePage(output, "static const Byte", partialName, reverseLookup.Select(value => value.ToString()), 16);
			}

			WriteCodePage(output, "const Byte *", "UnicodeTables_UnicodeTo" + codePageName + "Table", codePagePointers, 8);

			output.Append("const Int UnicodeTables_UnicodeTo" + codePageName
				+ "TableCount = sizeof(UnicodeTables_UnicodeTo" + codePageName
				+ "Table) / sizeof(const Byte **);\r\n");
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
