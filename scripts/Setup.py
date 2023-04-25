import os
import pathlib
import sys
import subprocess

compiler = "msvc"
argsLen = len(sys.argv)
if argsLen == 1:
	compiler = input("Select compiler: msvc/clang/llvm ")
elif argsLen == 2:
	compiler = sys.argv[1]
else:
	print("Invalid syntax, should only contain one argument: msvc/clang/llvm")
	exit

if compiler != "msvc" and compiler != "clang" and compiler != "llvm":
	print("Invalid compiler:", compiler)
	exit

if compiler == "clang" or compiler == "llvm":
	subprocess.call("GenerateSolutionClang.bat", stdin=subprocess.DEVNULL)
else:
	subprocess.call("GenerateSolution.bat", stdin=subprocess.DEVNULL)

if compiler == "llvm":
	llvmInstallPath = "C:/Program Files/LLVM/bin/clang.exe"
	if (not os.path.exists(llvmInstallPath)):
		print("LLVM installation not found!")
		subprocess.call("Install-LLVM.bat", stdin=subprocess.DEVNULL)
	subprocess.call("Install-LLVM-Utils.bat", stdin=subprocess.DEVNULL)
	for dname, dirs, files in os.walk("./.."):
		for fname in files:
			fpath = os.path.join(dname, fname)
			extension = pathlib.Path(fpath).suffix
			if extension != ".vcxproj":
				continue
			with open(fpath) as f:
				s = f.read()
			s = s.replace("<PlatformToolset>ClangCL</PlatformToolset>", "<PlatformToolset>LLVM_v143</PlatformToolset>")
			with open(fpath, "w") as f:
				f.write(s)
print("Generated project files for", compiler)
