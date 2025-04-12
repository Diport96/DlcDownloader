import glob
import hashlib
import json
import os
import pathlib
import re
from typing import List


def get_file_hash(file_path):
    sha1sum = hashlib.sha1()
    with open(file_path, 'rb') as source:
        block = source.read(2 ** 16)
        while len(block) != 0:
            sha1sum.update(block)
            block = source.read(2 ** 16)
    return sha1sum.hexdigest()


def create_json_data(dir_path: str, manifest_version: str, deployment_version: str) -> str:
    pak_dirs: List[str] = glob.glob(dir_path + "/**/*.pak", recursive=True)
    print(f"Found {len(pak_dirs)} Pak entries")
    sig_dirs: List[str] = glob.glob(dir_path + "/**/*.sig", recursive=True)
    print(f"Found {len(sig_dirs)} Signing entries")
    pak_files = []
    sig_files = []
    for item_dir in pak_dirs:
        file_name = os.path.basename(item_dir)
        relative_path = item_dir.replace(dir_path, "").replace("\\", "/")
        regex_chunk_id = re.search(r"pakchunk(\d*).*", file_name)
        chink_id: int = int(regex_chunk_id.group(1))
        file_size = os.path.getsize(item_dir)
        file_sha1 = get_file_hash(item_dir)
        pak_files.append({
            "LocalPath": file_name,
            "RemotePath": relative_path,
            "ChunkId": chink_id,
            "FileSize": file_size,
            "FileHash": f"SHA1:{file_sha1}"
        })
    for item_dir in sig_dirs:
        file_name = os.path.basename(item_dir)
        relative_path = item_dir.replace(dir_path, "").replace("\\", "/")
        file_size = os.path.getsize(item_dir)
        file_sha1 = get_file_hash(item_dir)
        sig_files.append({
            "LocalPath": file_name,
            "RemotePath": relative_path,
            "FileSize": file_size,
            "FileHash": f"SHA1:{file_sha1}"
        })
    obj = {
        "ManifestVersion": manifest_version,
        "DeploymentVersion": deployment_version,
        "PakFiles": pak_files,
        "SigFiles": sig_files
    }
    return json.dumps(obj, indent=4)


if __name__ == '__main__':
    local_path = pathlib.Path(__file__).parent.resolve()
    path_string: str = str(local_path.absolute())
    manifest_data = create_json_data(path_string, '1', '0.1')
    print(f"Manifest: {manifest_data}")

    manifest_file_name = "Example_Manifest.json"
    manifest_file_path = os.path.join(path_string, manifest_file_name)
    print(f"Creating/Opening {manifest_file_path}")
    file = open(manifest_file_path, 'w+')
    print(f"Writing data to {manifest_file_path}")
    file.write(manifest_data)
    file.close()
    print("Complete!")