using System.Collections.Generic;
using System.Text;
using System.Linq;

namespace UnicodeSetup
{
	public static class StringExtensions
	{
		public static string Join(this IEnumerable<string> pieces)
		{
			return Join(pieces, string.Empty);
		}

		public static string Join(this IEnumerable<string> pieces, string glue)
		{
			switch (pieces.Count())
			{
				case 0:
					return string.Empty;

				case 1:
					return pieces.First();

				case 2:
					string[] array = pieces.ToArray();
					return array[0] + glue + array[1];

				case 3:
					array = pieces.ToArray();
					return array[0] + glue + array[1] + glue + array[2];

				default:
					StringBuilder stringBuilder = new StringBuilder();
					bool isFirst = true;
					foreach (string piece in pieces)
					{
						if (!isFirst)
						{
							stringBuilder.Append(glue);
						}
						stringBuilder.Append(piece);
						isFirst = false;
					}
					return stringBuilder.ToString();
			}
		}
	}
}
