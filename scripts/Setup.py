import os
import pathlib
import sys
import requests
import time
import urllib

compiler = "msvc"
argsLen = len(sys.argv);
if argsLen == 1:
    print("Selecting default compiler msvc");
elif argsLen == 2:
    compiler = sys.argv[1]
else:
    print("Invalid syntax, should only contain one argument: msvc/clang")
    exit

if compiler != "msvc" and compiler != "clang":
    print("Invalid compiler:", compiler)
    exit

os.system("Win-GenProjects.bat")

def DownloadFile(url, filepath):
    path = filepath
    filepath = os.path.abspath(filepath)
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
            
    if (type(url) is list):
        for url_option in url:
            print("Downloading", url_option)
            try:
                DownloadFile(url_option, filepath)
                return
            except urllib.error.URLError as e:
                print(f"URL Error encountered: {e.reason}. Proceeding with backup...\n\n")
                os.remove(filepath)
                pass
            except urllib.error.HTTPError as e:
                print(f"HTTP Error  encountered: {e.code}. Proceeding with backup...\n\n")
                os.remove(filepath)
                pass
            except:
                print(f"Something went wrong. Proceeding with backup...\n\n")
                os.remove(filepath)
                pass
        raise ValueError(f"Failed to download {filepath}")
    if not(type(url) is str):
        raise TypeError("Argument 'url' must be of type list or string")

    with open(filepath, 'wb') as f:
        headers = {'User-Agent': "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36"}
        response = requests.get(url, headers=headers, stream=True)
        total = response.headers.get('content-length')

        if total is None:
            f.write(response.content)
        else:
            downloaded = 0
            total = int(total)
            startTime = time.time()
            for data in response.iter_content(chunk_size=max(int(total/1000), 1024*1024)):
                downloaded += len(data)
                f.write(data)
                
                try:
                    done = int(50*downloaded/total) if downloaded < total else 50
                    percentage = (downloaded / total) * 100 if downloaded < total else 100
                except ZeroDivisionError:
                    done = 50
                    percentage = 100
                elapsedTime = time.time() - startTime
                try:
                    avgKBPerSecond = (downloaded / 1024) / elapsedTime
                except ZeroDivisionError:
                    avgKBPerSecond = 0.0

                avgSpeedString = '{:.2f} KB/s'.format(avgKBPerSecond)
                if (avgKBPerSecond > 1024):
                    avgMBPerSecond = avgKBPerSecond / 1024
                    avgSpeedString = '{:.2f} MB/s'.format(avgMBPerSecond)
                sys.stdout.write('\r[{}{}] {:.2f}% ({})     '.format('█' * done, '.' * (50-done), percentage, avgSpeedString))
                sys.stdout.flush()
    sys.stdout.write('\n')

llvmVersion = "15.0.5"
llvmFileName = "LLVM-" + llvmVersion + "-win64.exe"
llvmDownloadUrl = "https://github.com/llvm/llvm-project/releases/download/llvmorg-" + llvmVersion + "/" + llvmFileName
llvmFileSaveLocation = ".cache/" + llvmFileName
if compiler == "clang":
    #DownloadFile(llvmDownloadUrl, llvmFileSaveLocation)
    #os.system("call \"" + llvmFileSaveLocation + "\"" + " /S")
    #print("Installing", llvmFileName)
    os.system("Install-LLVM.bat")
    os.system("Install-LLVM-Utils.bat")
    for dname, dirs, files in os.walk("./.."):
        for fname in files:
            fpath = os.path.join(dname, fname)
            extension = pathlib.Path(fpath).suffix
            if extension != ".vcxproj":
                continue
            with open(fpath) as f:
                s = f.read()
            s = s.replace("v143", "LLVM_v143")
            with open(fpath, "w") as f:
                f.write(s)
    print("Generated project files for", compiler)
