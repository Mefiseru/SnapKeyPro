import os
import platform
import subprocess
import sys

def install_python_requirements():
    """Install Python requirements using pip."""
    subprocess.check_call([sys.executable, "-m", "pip", "install", "--upgrade", "pip", "setuptools"])
    subprocess.check_call([sys.executable, "-m", "pip", "install", "-r", "requirements.txt"])

def install_system_packages_linux():
    """Install necessary system packages on Linux."""
    try:
        subprocess.check_call(["sudo", "apt-get", "update"])
        subprocess.check_call(["sudo", "apt-get", "install", "-y", "libx11-dev", "libxtst-dev"])
    except subprocess.CalledProcessError:
        print("Error installing system packages. Please make sure you have sudo privileges.")

def build_extension():
    """Build the C++ extension using setup.py."""
    try:
        subprocess.check_call([sys.executable, "setup.py", "build_ext", "--inplace"])
    except subprocess.CalledProcessError:
        print("Error building the C++ extension.")

def main():
    print("Setting up the SnapKeyPro project...")

    # Detect the operating system
    current_os = platform.system()
    print(f"Detected OS: {current_os}")

    # Install Python requirements
    print("Installing Python requirements...")
    install_python_requirements()

    # Install system packages if Linux
    if current_os == "Linux":
        print("Installing system packages for Linux...")
        install_system_packages_linux()

    # Build the C++ extension
    print("Building the C++ extension...")
    build_extension()

    print("Setup complete. You can now run the main.py script.")

if __name__ == "__main__":
    main()
