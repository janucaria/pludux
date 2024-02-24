import argparse
import os
import subprocess
import sys
import shlex


ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_DIR = ROOT_DIR + "/.out"


def err_print(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def print_run_command(command):
    err_print("run command: {}".format(shlex.join(command)))


def get_msvc_toolset_version():
    """Detect MSVC compiler version from VCToolsVersion environment variable."""
    vc_tools_version = os.getenv("VCToolsVersion")

    if vc_tools_version:
        # Extract minor version part to derive the MSVC version (e.g., "32" -> "193")
        version_major_minor = vc_tools_version.split(
            '.')[1]  # e.g., "32" in "14.32.31326"
        first_digit = version_major_minor[0]
        msvc_version = "19" + first_digit  # Convert to MSVC version format
        print(f"Detected MSVC version: {msvc_version}")
        return msvc_version
    else:
        raise EnvironmentError(
            "VCToolsVersion environment variable not found. Run in Developer Command Prompt.")


def create_dir_if_not_exists(dir_path):
    if not os.path.exists(dir_path):
        os.makedirs(dir_path)

    return dir_path


def create_conan_profile(conan_profile, build_type):
    msvc_version = get_msvc_toolset_version()

    with open(conan_profile, "w") as f:
        f.write(f"[settings]\n")
        f.write(f"os=Windows\n")
        f.write(f"arch=x86_64\n")
        f.write(f"compiler=msvc\n")
        f.write(f"compiler.version={msvc_version}\n")
        f.write(f"compiler.runtime=dynamic\n")
        f.write(f"build_type={build_type}\n")


def exec_conan_install(build_missing, conan_profile):
    command = ["conan",
               "install", ".",
               "--update",
               f"--output-folder={OUTPUT_DIR}",
               f"--profile:host={conan_profile}",
               f"--profile:build={conan_profile}"
               ]

    if build_missing:
        command.append("--build=missing")

    print_run_command(command)
    process_result = subprocess.run(command,
                                    universal_newlines=True)
    err_print("")

    return process_result.returncode


def run_prepare_deps(args):
    build_missing = args.build_missing
    build_type = args.build_type

    conan_profile_dir = create_dir_if_not_exists(
        f"{OUTPUT_DIR}/conan/profiles/")
    conan_profile = f"{conan_profile_dir}/{build_type}.txt"

    create_conan_profile(conan_profile, build_type)

    return_code = exec_conan_install(build_missing, conan_profile)
    if return_code:
        return return_code

    # print a success message with the conan file content
    err_print("Conan install success")
    with open(conan_profile, "r") as f:
        conan_profile_content = f.read()
        err_print("Conan profile content:")
        err_print(conan_profile_content)

    return return_code


def init_argparse() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers()

    configure_parser = subparsers.add_parser("prepare-deps")
    configure_parser.add_argument(
        "--build-missing",
        dest="build_missing",
        action="store_true",
        default=True,
        help="use the package if missing"
    )
    configure_parser.add_argument(
        "--build-type",
        dest="build_type",
        help="build type",
        choices=["Debug", "Release", "RelWithDebInfo", "MinSizeRel"],
        default="Debug"
    )
    configure_parser.set_defaults(handler=run_prepare_deps)

    return parser


def main() -> int:
    parser = init_argparse()
    args = parser.parse_args()

    if (args.handler != None):
        return args.handler(args)

    return 0


if __name__ == "__main__":
    error_code = main()
    sys.exit(error_code)
