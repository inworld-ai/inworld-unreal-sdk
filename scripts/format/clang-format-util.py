import os
import subprocess

os.chdir(os.path.dirname(os.path.abspath(__file__)))

style_file = os.path.join(os.getcwd(), ".clang-format")
inworld_source_path = os.path.join(os.getcwd(), "../../InworldAI/Source")

for subdir, dirs, files in os.walk(inworld_source_path):
    for file in files:
        if not file.endswith((".cpp", ".h", ".cs")):
            continue

        file_path = os.path.abspath(subdir + os.sep + file)

        command = r'clang-format -i -style=file:"{style_file}" {file_path}'.format(
            style_file=style_file,
            file_path=file_path,
        )

        if subprocess.run(command, shell=True).returncode != 0:
            raise SystemExit(
                "Error (command): failed to run {command}".format(command=command)
            )
