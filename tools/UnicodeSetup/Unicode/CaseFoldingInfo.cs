using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace UnicodeSetup.Unicode
{
	public class CaseFoldingInfo
	{
		public uint CodeValue;
		public CaseFoldingKind Kind;
		public List<uint> FoldedValues;

		#region Parsing

		private static readonly char[] _semicolonSplitter = new[] { ';' };
		private static readonly char[] _spaceSplitter = new[] { ' ' };

		public static CaseFoldingInfo Parse(string text)
		{
			string[] pieces = text.Split(_semicolonSplitter);

			uint codeValue = Convert.ToUInt32(pieces[0], 16);

			CaseFoldingKind kind;
			switch (pieces[1].Trim())
			{
				case "C":
					kind = CaseFoldingKind.Common;
					break;
				case "F":
					kind = CaseFoldingKind.Full;
					break;
				case "S":
					kind = CaseFoldingKind.Simple;
					break;
				case "T":
					kind = CaseFoldingKind.Turkic;
					break;
				default:
					throw new ArgumentException("Input line is not in a valid format.");
			}

			string[] values = pieces[2].Split(_spaceSplitter, StringSplitOptions.RemoveEmptyEntries);
			List<uint> foldedValues = values.Select(p => Convert.ToUInt32(p, 16)).ToList();

			CaseFoldingInfo info = new CaseFoldingInfo
			{
				CodeValue = codeValue,
				Kind = kind,
				FoldedValues = foldedValues,
			};

			return info;
		}

		#endregion

		#region Re-stringification

		public override string ToString()
		{
			return string.Format("{0:X4}; {1}; {2};",
				CodeValue,
				Kind == CaseFoldingKind.Common ? "C"
				: Kind == CaseFoldingKind.Full ? "F"
				: Kind == CaseFoldingKind.Simple ? "S"
				: Kind == CaseFoldingKind.Turkic ? "T"
				: string.Empty,
				FoldedValues.Select(f => f.ToString("X4")).Join(" ")
			);
		}

		#endregion
	}
}
