#!/usr/bin/env python3

import argparse
import os.path
import shutil
import subprocess

cmd_glslang = 'glslangValidator'
cmd_glslang_str = ' --spirv-val --glsl-version 450 -S {stage} -V {fin} -o {fout}'

def glslang_validator_exists():
    return shutil.which(cmd_glslang) is not None

def build_shader(filepath, sh_stage):
    path_out_pair = os.path.splitext(filepath)
    path_out = path_out_pair[0] + '.spv'
    # print('path_out: ', path_out)
    cmd_str = cmd_glslang_str.format(stage = sh_stage, fin = filepath, fout = path_out)
    cmd_str = cmd_glslang + cmd_str
    print('cmd_str: ', cmd_str)
    result = subprocess.call(cmd_str)
    print("return code: ", result)
    return 5

def build_shaderdir(dirpath):
    if not glslang_validator_exists():
        print(cmd_glslang, ' was not found in system PATH. Cannot execute.')
        return False

    f_vertex = os.path.join(dirpath, 'vertex.glsl')
    if not os.path.exists(f_vertex):
        print('vertex.glsl was not found')
        return False
    else:
        build_shader(f_vertex, 'vert')

    f_fragment = os.path.join(dirpath, 'fragment.glsl')
    if not os.path.exists(f_fragment):
        print('fragment.glsl was not found')
        return False
    else:
        build_shader(f_fragment, 'frag')


    return True

def main():
    # Parse arguments
    arg_parser = argparse.ArgumentParser(
        description='''vtek's shader compiler utility,
        calls glslangValidator behind the scenes to compile all
        GLSL shader files located in the directory given as
        argument.''')
    actions = arg_parser.add_argument_group('actions')
    actions.add_argument(
        '--clean', action='store_true',
        help='Delete compiled SPIR-V shader files in directory')
    actions.add_argument(
        'directory', nargs='?', help='shader directory to compile')

    args = arg_parser.parse_args()
    if not any([args.clean, args.directory]):
        arg_parser.error('no directory was provided')
    if not args.directory:
        print('No directory was provided')
        return

    # Locate target directory
    dirpath = os.path.join(os.getcwd(), args.directory)
    if not os.path.isdir(dirpath):
        print('Directory does not exist')
        return
    success = build_shaderdir(dirpath)

    # All went well
    if (success):
        return 0
    else:
        return 1


if __name__ == "__main__":
    main()
