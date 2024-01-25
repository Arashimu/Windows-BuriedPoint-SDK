import shutil
import os
import sys
import argparse

SCRIPT_PATH=os.path.split(os.path.realpath(__file__))[0]

BUILD_DIR_PATH=SCRIPT_PATH+'/../build'


def clear():
    if os.path.exists(BUILD_DIR_PATH):
        shutil.rmtree(BUILD_DIR_PATH)

def build_windows(platform='x64',config='Release',args=None):
    platform_dir=f'{BUILD_DIR_PATH}/{platform}-{config}'
    os.makedirs(platform_dir,exist_ok=True)
    
    os.chdir(platform_dir)
    
    build_cmd=f'cmake ../.. -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE={config} -DCMAKE_GENERATOR_PLATFORM={platform} -T v142'
    
    if args.test:
        build_cmd+=' -DBUILD_BURIED_TEST=ON'
    
    if args.example:
        build_cmd+=' -DBUILD_BURIED_EXAMPLES=ON'
    
    print("build command: "+build_cmd)
    
    ret=os.system(build_cmd)
    if ret!=0:
        print("build failed!!!!")
        return False

    build_cmd=f'cmake --build . --config {config} --parallel 8'
    ret=os.system(build_cmd)
    if ret!=0:
        print("build fail!!!")
        return False
    return True

def main():
    clear()
    os.makedirs(BUILD_DIR_PATH,exist_ok=True)
    parser=argparse.ArgumentParser(description="build windows")
    parser.add_argument('--test',action="store_true",default=False,help='run unittest')    
    parser.add_argument('--example',action="store_true",default=False,help='run examples')    
    args=parser.parse_args()
    if not build_windows(config='Debug',args=args):
        exit(1)

if __name__=='__main__':
    main()