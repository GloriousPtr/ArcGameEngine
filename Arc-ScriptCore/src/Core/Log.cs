using System.Diagnostics;
using System.Runtime.CompilerServices;
using JetBrains.Annotations;

namespace ArcEngine
{
	/// <summary>
	/// Class to handle logging
	/// </summary>
	[UsedImplicitly(ImplicitUseKindFlags.Default, ImplicitUseTargetFlags.WithMembers)]
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

		/// <summary>
		/// Log Trace.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Trace(object value, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
		{
			InternalCalls.Log_LogMessage(LogLevel.Trace, value != null ? value.ToString() : "null", filepath, memberName, line);
		}

		/// <summary>
		/// Log Debug.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Debug(object value, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
		{
			InternalCalls.Log_LogMessage(LogLevel.Debug, value != null ? value.ToString() : "null", filepath, memberName, line);
		}

		/// <summary>
		/// Log Info.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Info(object value, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
		{
			InternalCalls.Log_LogMessage(LogLevel.Info, value != null ? value.ToString() : "null", filepath, memberName, line);
		}

		/// <summary>
		/// Log Warn.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Warn(object value, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
		{
			InternalCalls.Log_LogMessage(LogLevel.Warn, value != null ? value.ToString() : "null", filepath, memberName, line);
		}

		/// <summary>
		/// Log Error.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Error(object value, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
		{
			InternalCalls.Log_LogMessage(LogLevel.Error, value != null ? value.ToString() : "null", filepath, memberName, line);
		}

		/// <summary>
		/// Log Critical.
		/// </summary>
		/// <param name="value">Object type that implements IEquatable</param>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static void Critical(object value, [CallerFilePath] string filepath = "", [CallerMemberName] string memberName = "", [CallerLineNumber] int line = 0)
		{
			InternalCalls.Log_LogMessage(LogLevel.Critical, value != null ? value.ToString() : "null", filepath, memberName, line);
		}
	}
}
