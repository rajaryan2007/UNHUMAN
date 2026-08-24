import json
import os

def generate_gun_gltf():
    # Load Box.gltf as base
    with open('../../sandbox/assets/models/Box.gltf', 'r') as f:
        data = json.load(f)

    # Modify nodes to make a gun shape
    # Node 0: Root
    # Node 1: Barrel (scaled Z, moved forward)
    # Node 2: Grip (scaled Y, moved down)
    
    data['scenes'][0]['nodes'] = [0]
    
    # Root node holds both parts
    data['nodes'] = [
        {
            "children": [1, 2],
            "name": "GunRoot"
        },
        {
            "mesh": 0,
            "name": "Barrel",
            "scale": [0.1, 0.1, 0.5],
            "translation": [0.0, 0.05, -0.2]
        },
        {
            "mesh": 0,
            "name": "Grip",
            "scale": [0.1, 0.3, 0.15],
            "translation": [0.0, -0.15, 0.0],
            # slight rotation for the grip
            "rotation": [0.1, 0.0, 0.0, 0.995]
        }
    ]
    
    # Modify the material to be dark grey/black
    data['materials'][0]['pbrMetallicRoughness']['baseColorFactor'] = [0.1, 0.1, 0.1, 1.0]
    data['materials'][0]['name'] = "GunMaterial"

    os.makedirs('../assets/models', exist_ok=True)
    with open('../assets/models/Gun.gltf', 'w') as f:
        json.dump(data, f, indent=4)
        
if __name__ == '__main__':
    generate_gun_gltf()
    print("Generated Gun.gltf successfully.")
