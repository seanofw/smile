using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace UnicodeSetup.Unicode
{
	public class SpecialCasingInfo
	{
		public uint CodeValue;
		public List<uint> Lowercase;
		public List<uint> Titlecase;
		public List<uint> Uppercase;
		public string Condition;

		#region Parsing

		private static readonly char[] _semicolonSplitter = new[] { ';' };
		private static readonly char[] _spaceSplitter = new[] { ' ' };

		public static SpecialCasingInfo Parse(string text)
		{
			string[] pieces = text.Split(_semicolonSplitter);

			uint codeValue = Convert.ToUInt32(pieces[0], 16);

			string[] values = pieces[1].Split(_spaceSplitter, StringSplitOptions.RemoveEmptyEntries);
			List<uint> lowercaseValues = values.Select(p => Convert.ToUInt32(p, 16)).ToList();

			values = pieces[2].Split(_spaceSplitter, StringSplitOptions.RemoveEmptyEntries);
			List<uint> titlecaseValues = values.Select(p => Convert.ToUInt32(p, 16)).ToList();

			values = pieces[3].Split(_spaceSplitter, StringSplitOptions.RemoveEmptyEntries);
			List<uint> uppercaseValues = values.Select(p => Convert.ToUInt32(p, 16)).ToList();

			SpecialCasingInfo info = new SpecialCasingInfo
			{
				CodeValue = codeValue,
				Lowercase = lowercaseValues,
				Titlecase = titlecaseValues,
				Uppercase = uppercaseValues,
				Condition = pieces[4].Trim(),
			};

			return info;
		}

		#endregion

		#region Re-stringification

		public override string ToString()
		{
			return string.Format("{0:X4}; {1}; {2}; {3}; {4}",
				CodeValue,
				Lowercase.Select(f => f.ToString("X4")).Join(" "),
				Titlecase.Select(f => f.ToString("X4")).Join(" "),
				Uppercase.Select(f => f.ToString("X4")).Join(" "),
				Condition
			);
		}

		#endregion
	}
}
