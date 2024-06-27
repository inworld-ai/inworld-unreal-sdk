import os
import re
import sys
import json
import stat
import getopt
import shutil
import subprocess

os.chdir(os.path.dirname(os.path.abspath(__file__)))

inworld_path = os.path.join(os.getcwd(), '../InworldAI')
temp_path = os.path.join(os.getcwd(), 'Temp')

if not os.path.exists(temp_path):
    os.makedirs(temp_path)

for tree in ['Config', 'Content', 'Resources', 'Source']:
    shutil.copytree(os.path.join(inworld_path, tree), os.path.join(temp_path, 'Copy', tree), dirs_exist_ok=True)
shutil.copy(os.path.join(inworld_path, 'InworldAI.uplugin'), os.path.join(temp_path, 'Copy', 'InworldAI.uplugin'))

for subdir, dirs, files in os.walk(os.path.join(temp_path, 'Copy', 'Source')):
    for file in files:
        if 'ThirdParty' in subdir:
            continue
        filepath = subdir + os.sep + file
        
        filedata = ''
        with open(filepath, 'r') as file:
            filedata = '// Copyright 2024 Theai, Inc. (DBA Inworld) All Rights Reserved.' + '\n' + re.sub('\/\*(.*?|\s)*\*\/', '', file.read())

        with open(filepath, 'w') as file:
            file.write(filedata)

commands = {
    'Win64': r'"C:\Program Files\Epic Games\UE_{unreal_version}\Engine\Build\BatchFiles\RunUAT.bat" BuildPlugin -plugin="{temp_path}\Copy\InworldAI.uplugin" -TargetPlatforms=Win64 -package="{output_dir}"',
    'Mac': r'"/Users/Shared/Epic Games/UE_{unreal_version}/Engine/Build/BatchFiles/RunUAT.sh" BuildPlugin -plugin="{temp_path}/Copy/InworldAI.uplugin" -TargetPlatforms=Mac -package="{output_dir}"'
}

unreal_versions = ['5.0', '5.1', '5.2', '5.3', '5.4']

platform = ''
unreal_version = ''
with_binaries = 'True'
def usage():
    print('[-p, --platform=] {platforms}'.format(platforms=list(commands.keys())))
    print('[-u, --unreal=] {unreal}'.format(unreal=unreal_versions))
    print('[-b, --binaries=] [True, False]')

try:
    opts, values = getopt.getopt(sys.argv[1:], "hp:u:b:", ['help', 'platform=', 'unreal=', 'binaries='])
except getopt.GetoptError as err:
    print(err)
    usage()
    raise SystemExit('Error: Unknown Argument.')
for opt, val in opts:
    if opt in ('-h', '--help'):
        usage()
        sys.exit()
    elif opt in ('-p', '--platform'):
        platform = val
    elif opt in ('-u', '--unreal'):
        unreal_version = val
    elif opt in ('-b', '--binaries'):
        with_binaries = val

if not platform in commands.keys():
    usage()
    raise SystemExit('Error (config): Missing platform.')

if not unreal_version in unreal_versions:
    usage()
    raise SystemExit('Error (config): Missing unreal version.')

output_dir = os.path.join(temp_path, 'InworldAI')
if not os.path.exists(output_dir):
    os.makedirs(output_dir)

command = commands[platform].format(unreal_version = unreal_version, temp_path = temp_path, output_dir = output_dir)
if subprocess.run(command, shell=True).returncode != 0:
    raise SystemExit('Error (command): failed to run {command}'.format(command = command))

def del_dir(path):
    if os.path.exists(path):
        for root, dirs, files in os.walk(path, topdown=False):
            for name in files:
                filename = os.path.join(root, name)
                os.chmod(filename, stat.S_IWUSR)
                os.remove(filename)
            for name in dirs:
                os.rmdir(os.path.join(root, name))
        os.rmdir(path) 

if with_binaries == 'False':
    del_dir(os.path.join(output_dir, 'Binaries'))
    del_dir(os.path.join(output_dir, 'Intermediate'))

inworld_version = ''
with open(os.path.join(output_dir, 'InworldAI.uplugin'), 'r') as file:
    inworld_version = json.loads(file.read())['VersionName']
shutil.make_archive('InworldAI' + '_' + inworld_version + '_' + unreal_version, 'zip', output_dir)

del_dir(temp_path)
