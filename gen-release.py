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

# InworldReadyPlayerMe
CreateDir("/InworldReadyPlayerMe")

CreateDir("/InworldReadyPlayerMe/gltfRuntime")
CopyFolder("/InworldReadyPlayerMe/gltfRuntime/Config")
CopyFolder("/InworldReadyPlayerMe/gltfRuntime/Content")
CopyFolder("/InworldReadyPlayerMe/gltfRuntime/Resources")
CopyFolder("/InworldReadyPlayerMe/gltfRuntime/Source")
CopyFile("/InworldReadyPlayerMe/gltfRuntime/gltfRuntime.uplugin")
CopyFile("/InworldReadyPlayerMe/gltfRuntime/README.md")
CopyFile("/InworldReadyPlayerMe/gltfRuntime/LICENSE")

CreateDir("/InworldReadyPlayerMe/InworldRPM")
CopyFolder("/InworldReadyPlayerMe/InworldRPM/Content")
CopyFolder("/InworldReadyPlayerMe/InworldRPM/Resources")
CopyFolder("/InworldReadyPlayerMe/InworldRPM/Source")
CopyFile("/InworldReadyPlayerMe/InworldRPM/InworldRPM.uplugin")

# zip
print("zip...")
PluginFile = open(CurDir + "/InworldAI/InworldAI.uplugin")
PluginData = json.load(PluginFile)

VerName = PluginData["VersionName"]
shutil.make_archive(VersionDir, 'zip', VersionDir)
