using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace UnicodeSetup
{
	public class CodePage : ICollection<string>, IEquatable<CodePage>
	{
		public uint ExternalIdentifier;

		private readonly List<string> _values = new List<string>();

		private int? _hashCode;

		public bool Equals(CodePage other)
		{
			if (other == null) return false;
			if (ReferenceEquals(other, this)) return true;

			if (_values.Count != other._values.Count) return false;

			if (GetHashCode() != other.GetHashCode()) return false;

			for (int i = 0; i < _values.Count; i++)
			{
				if (_values[i] != other._values[i]) return false;
			}

			return true;
		}

		public override bool Equals(object obj)
		{
			return Equals(obj as CodePage);
		}

		public override int GetHashCode()
		{
			if (_hashCode.HasValue) return _hashCode.Value;

			int hashCode = (int)_values.Count;

			foreach (string value in _values)
			{
				hashCode ^= value.GetHashCode();
			}

			_hashCode = hashCode;

			return hashCode;
		}

		public void Add(string value)
		{
			_values.Add(value);
			_hashCode = null;
		}

		public void Clear()
		{
			_values.Clear();
			_hashCode = null;
		}

		public bool Contains(string item)
		{
			return _values.Contains(item);
		}

		public void CopyTo(string[] array, int arrayIndex)
		{
			_values.CopyTo(array, arrayIndex);
		}

		public int Count
		{
			get
			{
				return _values.Count;
			}
		}

		public bool IsReadOnly
		{
			get { return false; }
		}

		public bool Remove(string item)
		{
			_hashCode = null;
			return _values.Remove(item);
		}

		public IEnumerator<string> GetEnumerator()
		{
			return _values.GetEnumerator();
		}

		System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
		{
			return _values.GetEnumerator();
		}
	}
}
