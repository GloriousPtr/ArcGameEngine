/*
	MIT License

	Copyright (c) Freya Holmér

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/
// Modified to work with ArcEngine
// Internal utility types, like non-allocating multi-return values.
// Lots of boring boilerplate in here~
// by Freya Holmér (https://github.com/FreyaHolmer/Mathfs)

using System;
using System.Collections;
using System.Collections.Generic;

namespace ArcEngine {

	/// <summary>Contains either 0, 1, 2 or 3 valid return values</summary>
	public readonly struct ResultsMax3<T> : IEnumerable<T> where T : struct {

		/// <summary>The number of valid values</summary>
		public readonly int count;

		/// <summary>The first value. This may or may not be set/defined - use .count to see how many are valid</summary>
		public readonly T a;

		/// <summary>The second value. This may or may not be set/defined - use .count to see how many are valid</summary>
		public readonly T b;

		/// <summary>The third value. This may or may not be set/defined - use .count to see how many are valid</summary>
		public readonly T c;

		/// <summary>Creates a result with three values</summary>
		/// <param name="a">The first value</param>
		/// <param name="b">The second value</param>
		/// <param name="c">The third value</param>
		public ResultsMax3( T a, T b, T c ) {
			this.a = a;
			this.b = b;
			this.c = c;
			count = 3;
		}

		/// <summary>Creates a result with two values</summary>
		/// <param name="a">The first value</param>
		/// <param name="b">The second value</param>
		public ResultsMax3( T a, T b ) {
			this.a = a;
			this.b = b;
			this.c = default;
			count = 2;
		}

		/// <summary>Creates a result with one value</summary>
		/// <param name="a">The one value</param>
		public ResultsMax3( T a ) {
			this.a = a;
			this.b = default;
			this.c = default;
			count = 1;
		}

		/// <summary>Returns the valid values at index i. Will throw an index out of range exception for invalid values. Use toghether with .count to ensure you don't get invalid values</summary>
		/// <param name="i">The index of the result to get</param>
		public T this[ int i ] {
			get {
				if( i >= 0 && i < count ) {
					switch( i ) {
						case 0: return a;
						case 1: return b;
						case 2: return c;
					}
				}

				throw new IndexOutOfRangeException();
			}
		}

		/// <summary>Returns a version of these results with one more element added to it. Note: this does not mutate the original struct</summary>
		/// <param name="value">The value to add</param>
		public ResultsMax3<T> Add( T value ) {
			switch( count ) {
				case 0:  return new ResultsMax3<T>( value );
				case 1:  return new ResultsMax3<T>( a, value );
				case 2:  return new ResultsMax3<T>( a, b, value );
				default: throw new IndexOutOfRangeException( "Can't add more than three values to ResultsMax3" );
			}
		}

		/// <summary>Implicitly casts a value to a results structure</summary>
		/// <param name="v">The value to cast</param>
		public static implicit operator ResultsMax3<T>( T v ) => new ResultsMax3<T>( v );

		/// <summary>Implicitly casts ResultsMax2 to ResultsMax3</summary>
		/// <param name="m2">The results to cast</param>
		public static implicit operator ResultsMax3<T>( ResultsMax2<T> m2 ) {
			switch( m2.count ) {
				case 0: return default;
				case 1: return new ResultsMax3<T>( m2.a );
				case 2: return new ResultsMax3<T>( m2.a, m2.b );
			}

			throw new InvalidCastException( "Failed to cast ResultsMax2 to ResultsMax3" );
		}
		
		/// <summary>Explicitly casts ResultsMax3 to ResultsMax2</summary>
		/// <param name="m3">The results to cast</param>
		public static explicit operator ResultsMax2<T>( ResultsMax3<T> m3 ) {
			switch( m3.count ) {
				case 0: return default;
				case 1: return new ResultsMax2<T>( m3.a );
				case 2: return new ResultsMax2<T>( m3.a, m3.b );
				case 3: throw new IndexOutOfRangeException( "Attempt to cast ResultsMax3 to ResultsMax2 when it had 3 results" );
			}

			throw new InvalidCastException( "Failed to cast ResultsMax2 to ResultsMax3" );
		}


		IEnumerator<T> IEnumerable<T>.GetEnumerator() => GetEnumerator();

		IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

		public struct ResultsMax3Enumerator : IEnumerator<T> {

			int currentIndex;
			readonly ResultsMax3<T> value;

			public ResultsMax3Enumerator( ResultsMax3<T> value ) {
				this.value = value;
				currentIndex = -1;
			}

			public bool MoveNext() => ++currentIndex < value.count;
			public void Reset() => currentIndex = -1;
			public T Current => value[currentIndex];
			object IEnumerator.Current => Current;
			public void Dispose() => _ = 0;
		}

		public ResultsMax3Enumerator GetEnumerator() => new ResultsMax3Enumerator( this );

	}

	/// <summary>Contains either 0, 1 or 2 valid return values</summary>
	public readonly struct ResultsMax2<T> : IEnumerable<T> where T : struct {

		/// <inheritdoc cref="Freya.ResultsMax3{T}.count"/>
		public readonly int count;

		/// <inheritdoc cref="Freya.ResultsMax3{T}.a"/>
		public readonly T a;

		/// <inheritdoc cref="Freya.ResultsMax3{T}.b"/>
		public readonly T b;

		/// <inheritdoc cref="Freya.ResultsMax3{T}(T,T)"/>
		public ResultsMax2( T a, T b ) {
			this.a = a;
			this.b = b;
			count = 2;
		}

		/// <inheritdoc cref="Freya.ResultsMax3{T}(T)"/>
		public ResultsMax2( T a ) {
			this.a = a;
			this.b = default;
			count = 1;
		}

		/// <inheritdoc cref="ResultsMax3{T}.this"/>
		public T this[ int i ] {
			get {
				if( i >= 0 && i < count ) {
					switch( i ) {
						case 0: return a;
						case 1: return b;
					}
				}

				throw new IndexOutOfRangeException();
			}
		}

		/// <inheritdoc cref="Freya.ResultsMax3{T}.Add(T)"/>
		public ResultsMax2<T> Add( T value ) {
			switch( count ) {
				case 0:  return new ResultsMax2<T>( value );
				case 1:  return new ResultsMax2<T>( a, value );
				default: throw new IndexOutOfRangeException( "Can't add more than two values to ResultsMax2" );
			}
		}


		IEnumerator<T> IEnumerable<T>.GetEnumerator() => GetEnumerator();

		IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();

		public struct ResultsMax2Enumerator : IEnumerator<T> {

			int currentIndex;
			readonly ResultsMax2<T> value;

			public ResultsMax2Enumerator( ResultsMax2<T> value ) {
				this.value = value;
				currentIndex = -1;
			}

			public bool MoveNext() => ++currentIndex < value.count;
			public void Reset() => currentIndex = -1;
			public T Current => value[currentIndex];
			object IEnumerator.Current => Current;
			public void Dispose() => _ = 0;
		}

		public ResultsMax2Enumerator GetEnumerator() => new ResultsMax2Enumerator( this );
	}

}
