import os
import stat
import contextlib

os.chdir(os.path.dirname(os.path.abspath(__file__)))

ndk_path = '../../../../inworld-ndk'
cpy_path = '..'

@contextlib.contextmanager
def in_path(path):
    prev_dir = os.getcwd()
    os.chdir(path)
    try:
        yield
    finally:
        os.chdir(prev_dir)

def del_dir(dir_to_del):
    full_path = os.path.join(os.getcwd(), dir_to_del)
    if os.path.exists(full_path):
        for root, dirs, files in os.walk(full_path, topdown=False):
            for name in files:
                filename = os.path.join(root, name)
                os.chmod(filename, stat.S_IWUSR)
                os.remove(filename)
            for name in dirs:
                os.rmdir(os.path.join(root, name))
        os.rmdir(full_path)  

with in_path(cpy_path):
    dirs_to_del = ['src', 'lib', 'include']
    for dir_to_del in dirs_to_del:
        del_dir(dir_to_del)
        
with in_path(ndk_path):
    del_dir('build')
