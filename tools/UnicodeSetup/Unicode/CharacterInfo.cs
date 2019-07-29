using System.Linq;
using System.Collections.Generic;
using System;

namespace UnicodeSetup.Unicode
{
	public class CharacterInfo
	{
		public uint CodeValue;
		public string CharacterName;
		public GeneralCategory GeneralCategory;
		public int CanonicalCombiningClass;
		public BidirectionalCategory BidirectionalCategory;
		public CharacterDecompositionMappingTag CharacterDecompositionMappingTag;
		public List<uint> CharacterDecompositionMapping;
		public int? DecimalDigitValue;
		public string DigitValue;
		public string NumericValue;
		public bool Mirrored;
		public string Unicode10Name;
		public string ISO10646Comment;
		public uint? UppercaseMapping;
		public uint? LowercaseMapping;
		public uint? TitlecaseMapping;

		#region Parsing

		private static readonly char[] _semicolonSplitter = new[] { ';' };
		private static readonly char[] _spaceSplitter = new[] { ' ' };

		public static CharacterInfo Parse(string line)
		{
			List<string> pieces = line.Split(_semicolonSplitter).ToList();
			while (pieces.Count < 14)
			{
				pieces.Add(string.Empty);
			}

			uint codeValue = Convert.ToUInt32(pieces[0], 16);

			CharacterDecompositionMappingTag characterDecompositionMappingTag;
			List<uint> characterDecompositionMapping;
			ParseCharacterDecompositionMapping(pieces[5], out characterDecompositionMappingTag, out characterDecompositionMapping);

			CharacterInfo characterInfo = new CharacterInfo
			{
				CodeValue = codeValue,
				CharacterName = pieces[1],
				GeneralCategory = (GeneralCategory)Enum.Parse(typeof(GeneralCategory), pieces[2], true),
				CanonicalCombiningClass = !string.IsNullOrEmpty(pieces[3]) ? int.Parse(pieces[3]) : 0,
				BidirectionalCategory = (BidirectionalCategory)Enum.Parse(typeof(BidirectionalCategory), pieces[4], true),
				CharacterDecompositionMappingTag = characterDecompositionMappingTag,
				CharacterDecompositionMapping = characterDecompositionMapping,
				DecimalDigitValue = !string.IsNullOrEmpty(pieces[6]) ? (int?)int.Parse(pieces[6]) : null,
				DigitValue = pieces[7],
				NumericValue = pieces[8],
				Mirrored = pieces[9] == "Y",
				Unicode10Name = pieces[10],
				ISO10646Comment = pieces[11],
				UppercaseMapping = !string.IsNullOrEmpty(pieces[12]) ? (uint?)Convert.ToUInt32(pieces[12], 16) : null,
				LowercaseMapping = !string.IsNullOrEmpty(pieces[13]) ? (uint?)Convert.ToUInt32(pieces[13], 16) : null,
				TitlecaseMapping = !string.IsNullOrEmpty(pieces[14]) ? (uint?)Convert.ToUInt32(pieces[14], 16) : null,
			};

			return characterInfo;
		}

		private static void ParseCharacterDecompositionMapping(string text, out CharacterDecompositionMappingTag characterDecompositionMappingTag,
			out List<uint> characterDecompositionMapping)
		{
			if (text.StartsWith("<"))
			{
				int tagEnd = text.IndexOf('>', 1);
				string tagText = text.Substring(1, tagEnd - 1);
				text = text.Substring(tagEnd + 1);
				characterDecompositionMappingTag = (CharacterDecompositionMappingTag)Enum.Parse(typeof(CharacterDecompositionMappingTag), tagText, true);
			}
			else
			{
				characterDecompositionMappingTag = CharacterDecompositionMappingTag.None;
			}

			string[] pieces = text.Split(_spaceSplitter, StringSplitOptions.RemoveEmptyEntries);

			characterDecompositionMapping = pieces.Select(p => Convert.ToUInt32(p, 16)).ToList();
		}

		#endregion

		#region Re-stringification

		public override string ToString()
		{
			string characterDecompositionMapping = CharacterDecompositionMapping.Any()
				? CharacterDecompositionMapping.Select(n => ((int)n).ToString("X4")).Join(" ")
				: string.Empty;
			if (CharacterDecompositionMappingTag != CharacterDecompositionMappingTag.None)
			{
				characterDecompositionMapping = "<" + CharacterDecompositionMappingTag.ToString() + "> " + characterDecompositionMapping;
			}

			return string.Format("CodeValue: \"{0:X4}\" \n"
				+ "CharacterName: \"{1}\", \n"
				+ "GeneralCategory: \"{2}\", \n"
				+ "CanonicalCombiningClass: \"{3}\", \n"
				+ "BidirectionalCategory: \"{4}\", \n"
				+ "CharacterDecompositionMapping: \"{5}\", \n"
				+ "DecimalDigitValue: \"{6}\", \n"
				+ "DigitValue: \"{7}\", \n"
				+ "NumericValue: \"{8}\", \n"
				+ "Mirrored: \"{9}\", \n"
				+ "Unicode10Name: \"{10}\", \n"
				+ "ISO10646Comment: \"{11}\", \n"
				+ "UppercaseMapping: \"{12}\", \n"
				+ "LowercaseMapping: \"{13}\", \n"
				+ "TitlecaseMapping: \"{14}\"\n",
				CodeValue,
				CharacterName,
				GeneralCategory,
				CanonicalCombiningClass,
				BidirectionalCategory != Unicode.BidirectionalCategory.None ? BidirectionalCategory.ToString() : string.Empty,
				characterDecompositionMapping,
				DecimalDigitValue.HasValue ? DecimalDigitValue.Value.ToString() : string.Empty,
				DigitValue,
				NumericValue,
				Mirrored ? "Y" : "N",
				Unicode10Name,
				ISO10646Comment,
				UppercaseMapping.HasValue ? UppercaseMapping.Value.ToString() : string.Empty,
				LowercaseMapping.HasValue ? LowercaseMapping.Value.ToString() : string.Empty,
				TitlecaseMapping.HasValue ? TitlecaseMapping.Value.ToString() : string.Empty
			);
		}

		public string ToUnicodeDataString()
		{
			string characterDecompositionMapping = CharacterDecompositionMapping.Any()
				? CharacterDecompositionMapping.Select(n => ((int)n).ToString("X4")).Join(" ")
				: string.Empty;
			if (CharacterDecompositionMappingTag != CharacterDecompositionMappingTag.None)
			{
				characterDecompositionMapping = "<" + CharacterDecompositionMappingTag.ToString() + "> " + characterDecompositionMapping;
			}

			return string.Format("{0:X4};{1};{2};{3};{4};{5};{6};{7};{8};{9};{10};{11};{12};{13};{14}",
				CodeValue,
				CharacterName,
				GeneralCategory,
				CanonicalCombiningClass,
				BidirectionalCategory != Unicode.BidirectionalCategory.None ? BidirectionalCategory.ToString() : string.Empty,
				characterDecompositionMapping,
				DecimalDigitValue.HasValue ? DecimalDigitValue.Value.ToString() : string.Empty,
				DigitValue,
				NumericValue,
				Mirrored ? "Y" : "N",
				Unicode10Name,
				ISO10646Comment,
				UppercaseMapping.HasValue ? UppercaseMapping.Value.ToString() : string.Empty,
				LowercaseMapping.HasValue ? LowercaseMapping.Value.ToString() : string.Empty,
				TitlecaseMapping.HasValue ? TitlecaseMapping.Value.ToString() : string.Empty
			);
		}

		#endregion
	}
}
