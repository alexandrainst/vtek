#!/usr/bin/env python3

import argparse
import os.path
import shutil
import subprocess

cmd_glslang = 'glslangValidator'
cmd_glslang_str = ' --spirv-val --glsl-version 450 -S {stage} -V {fin} -o {fout}'
spirv_validate = True
spirv_val_arg = '--spirv-val'
glsl_version = '450'

def glslang_validator_exists():
    return shutil.which(cmd_glslang) is not None


def build_shader(filepath, sh_stage):
    path_out_pair = os.path.splitext(filepath)
    path_out = path_out_pair[0] + '.spv'
    result = subprocess.call([
        cmd_glslang, spirv_val_arg, '--glsl-version', glsl_version, '-S', sh_stage, '-V', filepath, '-o', path_out])
    return result == 0


def build_shaderdir(dirpath):
    if not glslang_validator_exists():
        print(cmd_glslang, ' was not found in system PATH. Cannot execute.')
        return False

    result = 0

    # vertex
    f_vertex = os.path.join(dirpath, 'vertex.glsl')
    if os.path.exists(f_vertex):
        print("Compiling vertex shader..")
        result += abs(build_shader(f_vertex, 'vert'))

    # tessellation control
    f_tess_control = os.path.join(dirpath, 'tess_control.glsl')
    if os.path.exists(f_tess_control):
        print("Compiling tessellation control shader..")
        result += abs(build_shader(f_tess_control, 'tesc'))

    # tessellation evaluation
    f_tess_eval = os.path.join(dirpath, 'tess_eval.glsl')
    if os.path.exists(f_tess_eval):
        print("Compiling tessellation evaluation shader..")
        result += abs(build_shader(f_tess_eval, 'tese'))

    # geometry
    f_geometry = os.path.join(dirpath, 'geometry.glsl')
    if os.path.exists(f_geometry):
        print("Compiling geometry shader")
        result += abs(build_shader(f_geometry, 'geom'))

    # fragment
    f_fragment = os.path.join(dirpath, 'fragment.glsl')
    if os.path.exists(f_fragment):
        print("Compiling fragment shader..")
        result += abs(build_shader(f_fragment, 'frag'))

    return result == 0


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
