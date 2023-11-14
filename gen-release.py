import os
import shutil
import json

CurDir = os.getcwd()

PluginFile = open(CurDir + "/InworldAI/InworldAI.uplugin")
PluginData = json.load(PluginFile)
VerName = PluginData["VersionName"]
VersionDir = CurDir + "/package-release/inworld-unreal-sdk-" + VerName

ReleaseDir = VersionDir + "/InworldAI"

def CreateDir(Subdir):
    print("create directory " + Subdir)
    Dir = ReleaseDir + Subdir
    if not os.path.exists(Dir):
        os.mkdir(Dir)

def CopyFolder(Subdir):
    print("copy folder " + Subdir)
    os.mkdir(ReleaseDir + Subdir)
    shutil.copytree(CurDir + Subdir, ReleaseDir + Subdir, dirs_exist_ok=True)

def CopyFile(Subpath):
    print("copy file " + Subpath)
    shutil.copyfile(CurDir + Subpath, ReleaseDir + Subpath)

if os.path.exists(CurDir + "/package-release"):
    shutil.rmtree(CurDir + "/package-release")
os.mkdir(CurDir + "/package-release")
os.mkdir(VersionDir)
os.mkdir(ReleaseDir)

CopyFile("/LICENSE.md")
CopyFile("/README.md")
CopyFile("/Changelog.md")

# InworldAI
CreateDir("/InworldAI")

CopyFile("/InworldAI/InworldAI.uplugin")
CopyFile("/InworldAI/LICENSE.md")

CopyFolder("/InworldAI/Config")
CopyFolder("/InworldAI/Content")
CopyFolder("/InworldAI/Resources")
CopyFolder("/InworldAI/Source")

# inworld-ndk
CreateDir("/InworldAI/inworld-ndk")
CopyFolder("/InworldAI/inworld-ndk/src")

CreateDir("/InworldAI/inworld-ndk/ThirdParty")
CopyFolder("/InworldAI/inworld-ndk/ThirdParty/include")
CopyFolder("/InworldAI/inworld-ndk/ThirdParty/OpenSSL")
CopyFolder("/InworldAI/inworld-ndk/ThirdParty/Prebuilt")
CopyFolder("/InworldAI/inworld-ndk/ThirdParty/zlib")
CopyFile("/InworldAI/inworld-ndk/ThirdParty/README.md")

CreateDir("/InworldAI/inworld-ndk/ThirdParty/grpc")
CopyFolder("/InworldAI/inworld-ndk/ThirdParty/grpc/include")

CreateDir("/InworldAI/inworld-ndk/ThirdParty/grpc/third_party")
CopyFolder("/InworldAI/inworld-ndk/ThirdParty/grpc/third_party/abseil-cpp")

# InworldMetahuman
CreateDir("/InworldMetahuman")
CopyFolder("/InworldMetahuman/Content")
CopyFolder("/InworldMetahuman/Resources")
CopyFolder("/InworldMetahuman/Source")
CopyFile("/InworldMetahuman/InworldMetahuman.uplugin")

# InworldRPM
CreateDir("/InworldRPM")

CreateDir("/InworldRPM/gltfRuntime")
CopyFolder("/InworldRPM/gltfRuntime/Config")
CopyFolder("/InworldRPM/gltfRuntime/Content")
CopyFolder("/InworldRPM/gltfRuntime/Resources")
CopyFolder("/InworldRPM/gltfRuntime/Source")
CopyFile("/InworldRPM/gltfRuntime/gltfRuntime.uplugin")
CopyFile("/InworldRPM/gltfRuntime/README.md")
CopyFile("/InworldRPM/gltfRuntime/LICENSE")

CreateDir("/InworldRPM/InworldRPM")
CopyFolder("/InworldRPM/InworldRPM/Content")
CopyFolder("/InworldRPM/InworldRPM/Resources")
CopyFolder("/InworldRPM/InworldRPM/Source")
CopyFile("/InworldRPM/InworldRPM/InworldRPM.uplugin")

# zip
print("zip...")
shutil.make_archive(VersionDir, 'zip', VersionDir)
