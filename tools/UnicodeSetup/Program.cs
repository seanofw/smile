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
			new CSharp().GenerateAll();
			new CPlusPlus().GenerateAll();
		}
	}
}
