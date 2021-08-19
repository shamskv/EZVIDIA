import hashlib
import os
import json

manifest = []

files = [f for f in os.listdir('.') if os.path.isfile(f)]
for f in files:
    if(f == "manifest_generator.py"):
        continue
    m = hashlib.sha256()
    entry = {}
    entry["name"] = f
    with open(f, 'rb') as file_in:
        m.update(file_in.read())
        entry["hash"] = m.hexdigest()
    manifest.append(entry)

with open("manifest.json", 'w') as out:
    out.write(json.dumps(manifest))
