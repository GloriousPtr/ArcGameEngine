using System.Runtime.CompilerServices;

namespace ArcEngine
{
	public static class Log
	{
		internal enum LogLevel
		{
			Trace = (1 << 0),
			Debug = (1 << 1),
			Info = (1 << 2),
			Warn = (1 << 3),
			Error = (1 << 4),
			Critical = (1 << 5)
		}

		public static void Trace(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Trace, string.Format(format, parameters));
		public static void Debug(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Debug, string.Format(format, parameters));
		public static void Info(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Info, string.Format(format, parameters));
		public static void Warn(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Warn, string.Format(format, parameters));
		public static void Error(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Error, string.Format(format, parameters));
		public static void Critical(string format, params object[] parameters) =>InternalCalls.Log_LogMessage(LogLevel.Critical, string.Format(format, parameters));

		public static void Trace(object value) =>InternalCalls.Log_LogMessage(LogLevel.Trace, value != null ? value.ToString() : "null");
		public static void Debug(object value) =>InternalCalls.Log_LogMessage(LogLevel.Debug, value != null ? value.ToString() : "null");
		public static void Info(object value) =>InternalCalls.Log_LogMessage(LogLevel.Info, value != null ? value.ToString() : "null");
		public static void Warn(object value) =>InternalCalls.Log_LogMessage(LogLevel.Warn, value != null ? value.ToString() : "null");
		public static void Error(object value) =>InternalCalls.Log_LogMessage(LogLevel.Error, value != null ? value.ToString() : "null");
		public static void Critical(object value) =>InternalCalls.Log_LogMessage(LogLevel.Critical, value != null ? value.ToString() : "null");
	}
}
