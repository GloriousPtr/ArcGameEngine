using System;

namespace ArcEngine
{
    public static class ExceptionExtension
	{
		public static void ReportToArc(this Exception ex)
		{
			var lines = ex.ToString().SplitLines();

			foreach (var line in lines)
			{
				if (line.Line.IndexOf(":line") >= 0 && line.Line.IndexOf(") in ") >= 0 && line.Line.IndexOf(" at ") >= 0)
				{
					int filenameStart = line.Line.IndexOf(") in ") + 5;
					int filenameEnd = line.Line.LastIndexOf(":line ");
					ReadOnlySpan<char> filenameSpan = line.Line.Slice(filenameStart, filenameEnd - filenameStart);
					int lineNumberStart = filenameEnd + 6;
					int lineNumberEnd = line.Line.Length;
					ReadOnlySpan<char> lineNumberSpan = line.Line.Slice(lineNumberStart, lineNumberEnd - lineNumberStart);
					int lineNumber = int.Parse(lineNumberSpan);
					int methodNameStart = line.Line.IndexOf(" at ") + 4;
					int methodNameEnd = filenameStart - 5;
					ReadOnlySpan<char> methodNameSpan = line.Line.Slice(methodNameStart, methodNameEnd - methodNameStart);

					Log.Critical(ex.ToString(), filenameSpan.ToString(), methodNameSpan.ToString(), lineNumber);
					return;
				}
			}

			Log.Critical(ex.ToString(), "", "", 0);
		}
	}
}
