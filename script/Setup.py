import os
import sys
import platform
import urllib.request
import zipfile
import tarfile
from pathlib import Path

# Configuration
VENDOR_DIR = Path("UHE/vendor/bin")
OS_NAME = platform.system().lower() # 'windows' or 'linux'

# List your heavy binary dependencies here.
# You can add GLFW, Box2D, etc. later when you have their precompiled binaries hosted.
DEPENDENCIES = {
    "windows": {
        "slang": "https://github.com/shader-slang/slang/releases/download/v2026.10/slang-2026.10-windows-x86_64.zip"
    },
    "linux": {
        "slang": "https://github.com/shader-slang/slang/releases/download/v2026.10/slang-2026.10-linux-x86_64.zip"
    }
}

def report_hook(count, block_size, total_size):
    if total_size > 0:
        percent = int(count * block_size * 100 / total_size)
        sys.stdout.write(f"\rDownloading... {percent}%")
        sys.stdout.flush()

def extract_file(filepath, dest_dir):
    print(f"\nExtracting {filepath.name} to {dest_dir}...")
    if filepath.name.endswith('.zip'):
        with zipfile.ZipFile(filepath, 'r') as zip_ref:
            zip_ref.extractall(dest_dir)
    elif filepath.name.endswith('.tar.gz'):
        with tarfile.open(filepath, 'r:gz') as tar_ref:
            tar_ref.extractall(dest_dir)

def setup_dependencies():
    print(f"Detected OS: {OS_NAME}")
    
    if OS_NAME not in DEPENDENCIES:
        print(f"Error: Unsupported OS '{OS_NAME}'. Only Windows and Linux are supported.")
        sys.exit(1)
        
    os_deps = DEPENDENCIES[OS_NAME]
    target_bin_dir = VENDOR_DIR / OS_NAME
    target_bin_dir.mkdir(parents=True, exist_ok=True)
    
    for dep_name, url in os_deps.items():
        dep_dir = target_bin_dir / dep_name
        if dep_dir.exists():
            print(f"Dependency '{dep_name}' already exists at {dep_dir}. Skipping.")
            continue
            
        print(f"Fetching {dep_name} from {url}...")
        temp_file = target_bin_dir / url.split('/')[-1]
        
        try:
            urllib.request.urlretrieve(url, temp_file, reporthook=report_hook)
            extract_file(temp_file, dep_dir)
            temp_file.unlink() # Cleanup zip file
            print(f"Successfully installed {dep_name}.")
        except Exception as e:
            print(f"\nFailed to download or extract {dep_name}: {e}")
            sys.exit(1)

if __name__ == "__main__":
    # Ensure script is run from project root
    if not Path("UHE").exists():
        print("Error: Please run this script from the project root directory.")
        sys.exit(1)
        
    print("--- Starting UHE Dependency Setup ---")
    setup_dependencies()
    print("--- Setup Complete ---")
