
namespace UnicodeSetup.Unicode
{
	public enum GeneralCategory
	{
		// Normative Categories
		Cn = 0x00,
		Cc = 0x01,
		Cf = 0x02,
		Cs = 0x03,
		Co = 0x04,
		Lu = 0x11,
		Ll = 0x12,
		Lt = 0x13,
		Mn = 0x21,
		Mc = 0x22,
		Me = 0x23,
		Nd = 0x31,
		Nl = 0x32,
		No = 0x33,
		Zs = 0x41,
		Zl = 0x42,
		Zp = 0x43,

		// Informative Categories
		Lm = 0x19,
		Lo = 0x1A,
		Pc = 0x59,
		Pd = 0x5A,
		Ps = 0x5B,
		Pe = 0x5C,
		Pi = 0x5D,
		Pf = 0x5E,
		Po = 0x5F,
		Sm = 0x69,
		Sc = 0x6A,
		Sk = 0x6B,
		So = 0x6C,

		// 00 family: Others
		// 1x family: Letters
		// 2x family: Marks
		// 3x family: Numbers
		// 4x family: Separators
		// 5x family: Punctuation
		// 6x family: Spacing

		// 0..7 range: Normative
		// 8..F range: Informative
	}
}
