import getopt
import glob
import os
import subprocess
import sys

os.chdir(os.path.dirname(os.path.abspath(__file__)))

project_path = os.path.join(os.getcwd(), "../..")
project_file = os.path.abspath(glob.glob("../../*.uproject")[0])

commands = {
    "Win64": r'"C:\Program Files\Epic Games\UE_{unreal_version}\Engine\Binaries\Win64\UnrealEditor.exe" "{project_file}" -Game -nullrhi -ReportExportPath="{project_path}\Saved\Automation\Reports" -ExecCmds="Automation RunTests Inworld; Quit"'
}

unreal_versions = ["5.0", "5.1", "5.2", "5.3", "5.4"]

platform = ""
unreal_version = ""


def usage():
    print("[-p, --platform=] {platforms}".format(platforms=list(commands.keys())))
    print("[-u, --unreal=] {unreal}".format(unreal=unreal_versions))


try:
    opts, values = getopt.getopt(
        sys.argv[1:], "hp:u:b:", ["help", "platform=", "unreal="]
    )
except getopt.GetoptError as err:
    print(err)
    usage()
    raise SystemExit("Error: Unknown Argument.")
for opt, val in opts:
    if opt in ("-h", "--help"):
        usage()
        sys.exit()
    elif opt in ("-p", "--platform"):
        platform = val
    elif opt in ("-u", "--unreal"):
        unreal_version = val

if not platform in commands.keys():
    usage()
    raise SystemExit("Error (config): Missing platform.")

if not unreal_version in unreal_versions:
    usage()
    raise SystemExit("Error (config): Missing unreal version.")

command = commands[platform].format(
    unreal_version=unreal_version, project_file=project_file, project_path=project_path
)

if subprocess.run(command, shell=True).returncode != 0:
    raise SystemExit("Error (command): failed to run {command}".format(command=command))
