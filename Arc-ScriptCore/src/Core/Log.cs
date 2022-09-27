using System.Runtime.CompilerServices;

namespace ArcEngine
{
	/// <summary>
	/// Class to handle logging
	/// </summary>
	public static class Log
	{
		internal enum LogLevel
		{
			Trace		= (1 << 0),
			Debug		= (1 << 1),
			Info		= (1 << 2),
			Warn		= (1 << 3),
			Error		= (1 << 4),
			Critical	= (1 << 5)
		}

		#region Trace Overloads
		
		/// <summary>
		/// Log Trace.
		/// </summary>
		/// <param name="format">String format</param>
		/// <param name="parameters">Format parameters</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Trace(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Trace, string.Format(format, parameters));

		/// <summary>
		/// Log Trace.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Trace(object value) => InternalCalls.Log_LogMessage(LogLevel.Trace, value != null ? value.ToString() : "null");

		#endregion

		#region Debug Overloads

		/// <summary>
		/// Log Debug.
		/// </summary>
		/// <param name="format">String format</param>
		/// <param name="parameters">Format parameters</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Debug(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Debug, string.Format(format, parameters));

		/// <summary>
		/// Log Debug.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Debug(object value) => InternalCalls.Log_LogMessage(LogLevel.Debug, value != null ? value.ToString() : "null");

		#endregion

		#region Info Overloads

		/// <summary>
		/// Log Info.
		/// </summary>
		/// <param name="format">String format</param>
		/// <param name="parameters">Format parameters</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Info(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Info, string.Format(format, parameters));

		/// <summary>
		/// Log Info.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Info(object value) => InternalCalls.Log_LogMessage(LogLevel.Info, value != null ? value.ToString() : "null");

		#endregion

		#region Warn Overloads

		/// <summary>
		/// Log Warn.
		/// </summary>
		/// <param name="format">String format</param>
		/// <param name="parameters">Format parameters</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Warn(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Warn, string.Format(format, parameters));

		/// <summary>
		/// Log Warn.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Warn(object value) => InternalCalls.Log_LogMessage(LogLevel.Warn, value != null ? value.ToString() : "null");

		#endregion

		#region Error Overloads

		/// <summary>
		/// Log Error.
		/// </summary>
		/// <param name="format">String format</param>
		/// <param name="parameters">Format parameters</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Error(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Error, string.Format(format, parameters));

		/// <summary>
		/// Log Error.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Error(object value) => InternalCalls.Log_LogMessage(LogLevel.Error, value != null ? value.ToString() : "null");

		#endregion

		#region Critical Overloads

		/// <summary>
		/// Log Critical.
		/// </summary>
		/// <param name="format">String format</param>
		/// <param name="parameters">Format parameters</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Critical(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Critical, string.Format(format, parameters));

		/// <summary>
		/// Log Critical.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Critical(object value) =>InternalCalls.Log_LogMessage(LogLevel.Critical, value != null ? value.ToString() : "null");

		#endregion
	}
}
