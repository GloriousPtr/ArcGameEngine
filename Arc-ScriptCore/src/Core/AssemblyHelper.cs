using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace ArcEngine
{
	//[DebuggerStepThrough]
	internal static class AssemblyHelper
	{
		internal static AssemblyLoadContext m_AssemblyContext;
		internal static Dictionary<ulong, Assembly> m_Assemblies = new Dictionary<ulong, Assembly>();
		internal static HashSet<IntPtr> m_Allocations = new HashSet<IntPtr>();
		internal static Dictionary<IntPtr, IntPtr[]> m_ArrayAllocations = new Dictionary<IntPtr, IntPtr[]>();
		internal static Dictionary<ulong, MethodInfo> m_Methods = new Dictionary<ulong, MethodInfo>();

		internal static BindingFlags InstanceAll = BindingFlags.Instance | BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.FlattenHierarchy;

		internal static ulong UUID() => BitConverter.ToUInt64(Guid.NewGuid().ToByteArray(), 0);

		internal static Assembly LoadAssemblyWithSymbols(AssemblyLoadContext ctx, string dllPath)
		{
			string pdbPath = Path.Combine(Path.GetDirectoryName(dllPath), Path.GetFileNameWithoutExtension(dllPath) + ".pdb");

			byte[] dll = File.ReadAllBytes(dllPath);
			MemoryStream dllStream = new MemoryStream(dll);
			MemoryStream pdbStream = null;
			if (Path.Exists(pdbPath))
			{
				byte[] pdb = File.ReadAllBytes(pdbPath);
				pdbStream = new MemoryStream(pdb);
			}
			return ctx.LoadFromStream(dllStream, pdbStream);
		}

		internal static ulong LoadAssemblyUsingPath(string path)
		{
			if (m_AssemblyContext == null)
			{
				m_AssemblyContext = new AssemblyLoadContext(null, true);
				m_AssemblyContext.Resolving += (AssemblyLoadContext ctx, AssemblyName name) =>
				{
					string dependencyPath = Path.Combine(Path.GetDirectoryName(path), name.Name + ".dll");
					return LoadAssemblyWithSymbols(ctx, dependencyPath);
				};
			}

			string asmName = Path.GetFileNameWithoutExtension(path);
			foreach (var asm in m_AssemblyContext.Assemblies)
			{
				if (asm.FullName == asmName)
				{
					Log.Critical(string.Format("Assembly with the same name: {0} is already loaded", asmName));
					return 0;
				}
			}

			ulong id = UUID();
			Assembly assembly = LoadAssemblyWithSymbols(m_AssemblyContext, path);
			m_Assemblies.Add(id, assembly);
			return id;
		}

		[UnmanagedCallersOnly]
		internal static IntPtr LoadAssembly(IntPtr assemblyPath)
		{
			try
			{
				string asmPath = Marshal.PtrToStringAnsi(assemblyPath);
				return (IntPtr)LoadAssemblyUsingPath(asmPath);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetClassNames(IntPtr assembly, IntPtr parentAssembly, IntPtr parentClass)
		{
			try
			{
				if (assembly == IntPtr.Zero)
					return IntPtr.Zero;

				bool hasAsm = m_Assemblies.TryGetValue((ulong)assembly, out Assembly asm);
				if (!hasAsm)
					return IntPtr.Zero;

				Type[] types = asm.GetTypes();

				if (parentAssembly == IntPtr.Zero || parentClass == IntPtr.Zero)
				{
					string[] strings = new string[types.Length];
					for (int i = 0; i < strings.Length; ++i)
						strings[i] = types[i].FullName;

					return AllocateStringArray(strings);
				}
				else
				{
					bool hasParentAsm = m_Assemblies.TryGetValue((ulong)parentAssembly, out Assembly parentAsm);
					if (!hasParentAsm)
						return IntPtr.Zero;

					string parentName = Marshal.PtrToStringAnsi(parentClass);
					if (!string.IsNullOrEmpty(parentName))
					{
						Type parentType = parentAsm.GetType(parentName);
						if (parentType != null)
						{
							List<string> strings = new List<string>();
							foreach (Type type in types)
							{
								if (type.IsSubclassOf(parentType))
									strings.Add(type.FullName);
							}

							return AllocateStringArray(strings.ToArray());
						}
					}
				}

				return IntPtr.Zero;
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static void Free(IntPtr ptr)
		{
			try
			{
				if (m_Allocations.Contains(ptr))
				{
					Marshal.FreeHGlobal(ptr);
					m_Allocations.Remove(ptr);
				}
				else
				{
					Log.Critical(string.Format("Deallocating handle ({0}) not found", ptr.ToString()));
				}
			}
			catch (Exception e)
			{
				Log.Critical(string.Format("Deallocating handle ({0}) Exception: {1}", ptr.ToString(), e.ToString()));
			}
		}

		[UnmanagedCallersOnly]
		internal static void FreeArray(IntPtr ptr)
		{
			try
			{
				if (m_ArrayAllocations.TryGetValue(ptr, out IntPtr[] ptrs))
				{
					m_ArrayAllocations.Remove(ptr);
					Marshal.FreeHGlobal(ptr);
					foreach (IntPtr p in ptrs)
						Marshal.FreeHGlobal(p);
				}
				else
				{
					Log.Critical(string.Format("Deallocating handle ({0}) not found", ptr.ToString()));
				}
			}
			catch (Exception e)
			{
				Log.Critical(string.Format("Deallocating handle ({0}) Exception: {1}", ptr.ToString(), e.ToString()));
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr CreateEntityReference(IntPtr assembly, IntPtr className, ulong entityId)
		{
			try
			{
				bool hasAsm = m_Assemblies.TryGetValue((ulong)assembly, out Assembly asm);
				if (!hasAsm)
					return IntPtr.Zero;

				string classNameString = Marshal.PtrToStringAnsi(className);
				Type type = asm.GetType(classNameString);
				if (type == null)
					return IntPtr.Zero;

				object entity = Activator.CreateInstance(type);
				PropertyInfo property = type.GetProperty("ID", InstanceAll);
				property.SetValue(entity, entityId);
				GCHandle handle = GCHandle.Alloc(entity, GCHandleType.Normal);
				return GCHandle.ToIntPtr(handle);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr CreateObjectReference(IntPtr assembly, IntPtr className)
		{
			try
			{
				bool hasAsm = m_Assemblies.TryGetValue((ulong)assembly, out Assembly asm);
				if (!hasAsm)
					return IntPtr.Zero;

				string classNameString = Marshal.PtrToStringAnsi(className);
				Type type = asm.GetType(classNameString);
				if (type == null)
					return IntPtr.Zero;

				object obj = Activator.CreateInstance(type);
				GCHandle handle = GCHandle.Alloc(obj, GCHandleType.Normal);
				return GCHandle.ToIntPtr(handle);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static void ReleaseObjectReference(IntPtr objectPtr)
		{
			try
			{
				if (objectPtr != IntPtr.Zero)
				{
					GCHandle handle = GCHandle.FromIntPtr(objectPtr);
					handle.Free();
				}
				else
				{
					Log.Critical(string.Format("Deallocation handle ({0}) not found", objectPtr.ToString()));
				}
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetMethod(IntPtr objectPtr, IntPtr methodName)
		{
			try
			{
				if (objectPtr == IntPtr.Zero || methodName == IntPtr.Zero)
					return IntPtr.Zero;

				GCHandle handle = GCHandle.FromIntPtr(objectPtr);
				if (handle == null || handle.Target == null)
					return IntPtr.Zero;

				string methodNameString = Marshal.PtrToStringAnsi(methodName);
				MethodInfo method = handle.Target.GetType().GetMethod(methodNameString, InstanceAll);
				if (method == null)
					return IntPtr.Zero;

				ulong id = UUID();
				m_Methods[id] = method;
				return (IntPtr)id;
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static void InvokeMethod(IntPtr objectPtr, IntPtr methodId, IntPtr parameters, int parameterCount)
		{
			try
			{
				if (objectPtr == IntPtr.Zero || methodId == IntPtr.Zero)
					return;

				GCHandle handle = GCHandle.FromIntPtr(objectPtr);
				if (handle == null || handle.Target == null)
					return;

				MethodInfo method = m_Methods[(ulong)methodId];

				object[] obj = null;
				if (parameters != IntPtr.Zero)
				{
					ParameterInfo[] methodParams = method.GetParameters();
					int count = Math.Min(methodParams.Length, parameterCount);
					obj = new object[count];
					for (int i = 0; i < count; i++)
						obj[i] = Marshal.PtrToStructure(Marshal.ReadIntPtr(parameters + (i * IntPtr.Size)), methodParams[i].ParameterType);
				}

				method.Invoke(handle.Target, obj);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
			}
		}

		[UnmanagedCallersOnly]
		internal static int IsDebuggerAttached()
		{
			return Debugger.IsAttached ? 1 : 0;
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetTypeFromName(IntPtr assembly, IntPtr typeName)
		{
			try
			{
				bool hasAsm = m_Assemblies.TryGetValue((ulong)assembly, out Assembly asm);
				if (!hasAsm)
					return IntPtr.Zero;

				string typeNameString = Marshal.PtrToStringAnsi(typeName);
				Type type = asm.GetType(typeNameString);

				if (type == null)
					return IntPtr.Zero;
				else
					return type.TypeHandle.Value;
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetNameFromType(IntPtr typeHandle)
		{
			try
			{
				RuntimeTypeHandle handle = RuntimeTypeHandle.FromIntPtr(typeHandle);
				Type type = Type.GetTypeFromHandle(handle);

				if (type == null)
					return IntPtr.Zero;
			
				return AllocateString(type.FullName);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static void UnloadAssemblies()
		{

			if (m_Allocations.Count > 0)
			{
				Log.Critical("Memory leak found!");
				try
				{
					foreach (IntPtr p in m_Allocations)
						Marshal.FreeHGlobal(p);
				}
				catch (Exception e)
				{
					Log.Critical(string.Format("Deallocating Handle Exception: {0}", e.ToString()));
				}
				m_Allocations.Clear();
			}

			if (m_ArrayAllocations.Count > 0)
			{
				Log.Critical("Memory leak found!");
				try
				{
					foreach (var (ptr, array) in m_ArrayAllocations)
					{
						Marshal.FreeHGlobal(ptr);
						foreach (IntPtr p in array)
							Marshal.FreeHGlobal(p);
					}
				}
				catch (Exception e)
				{
					Log.Critical(string.Format("Deallocating Handle Exception: {0}", e.ToString()));
				}
				m_ArrayAllocations.Clear();

			}

			try
			{
				m_Assemblies.Clear();
				m_AssemblyContext?.Unload();
				m_AssemblyContext = null;
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
			}
		}

		internal static FieldInfo GetFieldInfo(IntPtr assembly, IntPtr className, IntPtr fieldName)
		{
			if (assembly == IntPtr.Zero || className == IntPtr.Zero || fieldName == IntPtr.Zero)
				return null;

			bool hasAsm = m_Assemblies.TryGetValue((ulong)assembly, out Assembly asm);
			if (!hasAsm)
				return null;

			string classNameString = Marshal.PtrToStringAnsi(className);
			Type type = asm.GetType(classNameString);
			if (type == null)
				return null;

			string fieldNameString = Marshal.PtrToStringAnsi(fieldName);
			FieldInfo field = type.GetField(fieldNameString, InstanceAll);
			return field;
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetFieldNames(IntPtr assembly, IntPtr className)
		{
			try
			{
				if (assembly == IntPtr.Zero || className == IntPtr.Zero)
					return IntPtr.Zero;

				bool hasAsm = m_Assemblies.TryGetValue((ulong)assembly, out Assembly asm);
				if (!hasAsm)
					return IntPtr.Zero;

				string classNameString = Marshal.PtrToStringAnsi(className);
				Type type = asm.GetType(classNameString);
				if (type == null)
					return IntPtr.Zero;

				FieldInfo[] fields = type.GetFields(InstanceAll);
				string[] strings = new string[fields.Length];
				for (int i = 0; i < fields.Length; ++i)
					strings[i] = fields[i].Name;

				return AllocateStringArray(strings);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetFieldTypeName(IntPtr assembly, IntPtr className, IntPtr fieldName)
		{
			try
			{
				FieldInfo fieldInfo = GetFieldInfo(assembly, className, fieldName);
				if (fieldInfo == null)
					return IntPtr.Zero;

				return AllocateString(fieldInfo.FieldType.FullName);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetFieldDisplayName(IntPtr assembly, IntPtr className, IntPtr fieldName)
		{
			try
			{
				FieldInfo fieldInfo = GetFieldInfo(assembly, className, fieldName);
				if (fieldInfo == null)
					return IntPtr.Zero;

				var attr = fieldInfo.CustomAttributes.FirstOrDefault(x => x.AttributeType.FullName == typeof(HeaderAttribute).FullName);
				if (attr == null)
					return IntPtr.Zero;

				return AllocateString(attr.ConstructorArguments[0].Value as string);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetFieldTooltip(IntPtr assembly, IntPtr className, IntPtr fieldName)
		{
			try
			{
				FieldInfo fieldInfo = GetFieldInfo(assembly, className, fieldName);
				if (fieldInfo == null)
					return IntPtr.Zero;

				var attr = fieldInfo.CustomAttributes.FirstOrDefault(x =>  x.AttributeType.FullName == typeof(TooltipAttribute).FullName);
				if (attr == null)
					return IntPtr.Zero;

				return AllocateString(attr.ConstructorArguments[0].Value as string);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static int IsFieldPublic(IntPtr assembly, IntPtr className, IntPtr fieldName)
		{
			try
			{
				FieldInfo fieldInfo = GetFieldInfo(assembly, className, fieldName);
				if (fieldInfo == null)
					return 0;

				return fieldInfo.Attributes.HasFlag(FieldAttributes.Public) ? 1 : 0;
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return 0;
			}
		}

		[UnmanagedCallersOnly]
		internal static int FieldHasAttribute(IntPtr assembly, IntPtr className, IntPtr fieldName, IntPtr attrName)
		{
			try
			{
				if (attrName == IntPtr.Zero)
					return 0;

				FieldInfo fieldInfo = GetFieldInfo(assembly, className, fieldName);
				if (fieldInfo == null)
					return 0;

				string attrNameString = Marshal.PtrToStringAnsi(attrName);
				foreach (var attr in fieldInfo.CustomAttributes)
				{
					if (string.Equals(attrNameString, attr.AttributeType.FullName))
						return 1;
				}

				return 0;
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return 0;
			}
		}

		[UnmanagedCallersOnly]
		internal static FloatRange GetFieldRange(IntPtr assembly, IntPtr className, IntPtr fieldName)
		{
			try
			{
				FieldInfo fieldInfo = GetFieldInfo(assembly, className, fieldName);
				if (fieldInfo == null)
					return new FloatRange(0.0f, 0.0f);

				var attr = fieldInfo.CustomAttributes.FirstOrDefault(x => x.AttributeType.FullName == typeof(RangeAttribute).FullName);
				if (attr == null)
					return new FloatRange(0.0f, 0.0f);

				return new FloatRange((float)attr.ConstructorArguments[0].Value, (float)attr.ConstructorArguments[1].Value);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return new FloatRange(0.0f, 0.0f);
			}
		}

		[UnmanagedCallersOnly]
		internal static void GetFieldValue(IntPtr objectPtr, IntPtr fieldName, IntPtr dataOut)
		{
			try
			{
				if (objectPtr == IntPtr.Zero || dataOut == IntPtr.Zero)
					return;

				GCHandle handle = GCHandle.FromIntPtr(objectPtr);
				object obj = handle.Target;
				if (obj == null)
					return;

				string fieldNameString = Marshal.PtrToStringAnsi(fieldName);
				FieldInfo fieldInfo = obj.GetType().GetField(fieldNameString, InstanceAll);
				if (fieldInfo == null)
					return;

				object value = fieldInfo.GetValue(obj);
				if (value != null)
				{
					if (value.GetType() == typeof(bool))
						Marshal.WriteByte(dataOut, (byte)((bool)value ? 1 : 0));
					else if (fieldInfo.FieldType == typeof(string))
						dataOut = AllocateString(value as string);
					else
						Marshal.StructureToPtr(value, dataOut, false);
				}
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
			}
		}

		[UnmanagedCallersOnly]
		internal static IntPtr GetFieldValueString(IntPtr objectPtr, IntPtr fieldName)
		{
			try
			{
				if (objectPtr == IntPtr.Zero)
					return IntPtr.Zero;

				GCHandle handle = GCHandle.FromIntPtr(objectPtr);
				object obj = handle.Target;
				if (obj == null)
					return IntPtr.Zero;

				string fieldNameString = Marshal.PtrToStringAnsi(fieldName);
				FieldInfo fieldInfo = obj.GetType().GetField(fieldNameString, InstanceAll);
				if (fieldInfo == null)
					return IntPtr.Zero;

				if (fieldInfo.FieldType != typeof(string))
					return IntPtr.Zero;

				object value = fieldInfo.GetValue(obj);
				if (value == null)
					return IntPtr.Zero;

				return AllocateString(value as string);
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
				return IntPtr.Zero;
			}
		}

		[UnmanagedCallersOnly]
		internal static void SetFieldValue(IntPtr objectPtr, IntPtr fieldName, IntPtr data)
		{
			try
			{
				if (objectPtr == IntPtr.Zero || data == IntPtr.Zero)
					return;

				GCHandle handle = GCHandle.FromIntPtr(objectPtr);
				object obj = handle.Target;
				if (obj == null)
					return;

				string fieldNameString = Marshal.PtrToStringAnsi(fieldName);
				FieldInfo fieldInfo = obj.GetType().GetField(fieldNameString, InstanceAll);
				if (fieldInfo == null)
					return;

				if (fieldInfo.FieldType == typeof(bool))
					fieldInfo.SetValue(obj, Marshal.ReadByte(data) == 0 ? false : true);
				else if (fieldInfo.FieldType == typeof(string))
					fieldInfo.SetValue(obj, Marshal.PtrToStringAnsi(data));
				else
					fieldInfo.SetValue(obj, Marshal.PtrToStructure(data, fieldInfo.FieldType));
			}
			catch (Exception ex)
			{
				ex.ReportToArc();
			}
		}

		internal static IntPtr AllocateString(string value)
		{
			IntPtr ptr = Marshal.StringToHGlobalAnsi(value);
			m_Allocations.Add(ptr);
			return ptr;
		}

		internal static IntPtr AllocateStringArray(string[] value)
		{
			int count = value.Length + 1;
			IntPtr[] strings = new IntPtr[count];
			for (int i = 0; i < value.Length; ++i)
				strings[i] = Marshal.StringToHGlobalAnsi(value[i]);
			strings[value.Length] = IntPtr.Zero;

			IntPtr result = Marshal.AllocHGlobal(IntPtr.Size * count);
			Marshal.Copy(strings, 0, result, count);

			if (!m_ArrayAllocations.ContainsKey(result))
				m_ArrayAllocations.Add(result, strings);

			return result;
		}

	}
}
