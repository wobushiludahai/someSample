import os
 
def execute(file):
    os.system("python3 -u " + file)

def get_current_file_path():
    return os.path.abspath(__file__)

def get_current_dir():
    return os.path.dirname(os.path.realpath(__file__))

def list_all_files(directory):
    print(directory)
    dir = os.listdir(directory)
    for file in dir:
        execute(directory + '/' + file)

cur_dir = get_current_dir()
list_all_files(cur_dir + '/examples')