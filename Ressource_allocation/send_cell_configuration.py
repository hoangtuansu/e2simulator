import json
import requests
import os
# Charger les politiques depuis le fichier

json_path = os.path.expanduser('/home/jordan/e2sim/e2sim/kpm_e2sm/cellmeasreport.json')
with open(json_path, 'r') as file:

    policies_data = json.load(file)

allocation_policies = policies_data.get('Allocation Policies', [])
qos_requirements = {item['UE Type']: item['Minimum QoS Requirement'] for item in policies_data.get('QoS Requirements', [])}

# Adresse E2SIM (adapter au besoin)
E2SIM_URL = "http://localhost:8080/api/v1.0/cell_configuration"

# Générer une requête pour chaque politique
for policy in allocation_policies:
    policy_name = policy['Policy Name']
    description = policy['Description']

    # Exemple générique de configuration (à adapter selon ton ASN.1 / modèle)
    cell_config_request = {
        "policyName": policy_name,
        "description": description,
        "parameters": {}
    }

    # Ajouter des paramètres spécifiques selon la politique
    if "Equal Allocation" in policy_name:
        cell_config_request['parameters'] = {
            "voiceShare": 0.5,
            "mbbShare": 0.5,
            "qosVoice": qos_requirements.get('Voice', '250 Kbps'),
            "qosMBB": qos_requirements.get('MBB', '3 Gbps')
        }
    elif "Voice Priority" in policy_name:
        cell_config_request['parameters'] = {
            "priority": "voice",
            "voiceMultiplier": 2,
            "qosVoice": qos_requirements.get('Voice')
        }
    elif "MBB Priority" in policy_name:
        cell_config_request['parameters'] = {
            "priority": "mbb",
            "mbbMultiplier": 2,
            "qosMBB": qos_requirements.get('MBB')
        }
    elif "Dedicated Resources" in policy_name:
        cell_config_request['parameters'] = {
            "voiceReservation": 0.3,  # alpha = 30%
            "mbbReservation": 0.7,    # beta = 70%
            "qosVoice": qos_requirements.get('Voice'),
            "qosMBB": qos_requirements.get('MBB')
        }

    # Afficher la requête
    print(f"\n📤 Sending CellConfigurationRequest for: {policy_name}")
    print(json.dumps(cell_config_request, indent=2))

    # Envoyer la requête à E2SIM
    try:
        response = requests.post(E2SIM_URL, json=cell_config_request, timeout=5)
        if response.status_code == 200:
            print("✅ Successfully sent!\n")
        else:
            print(f"❌ Failed to send: {response.status_code} - {response.text}\n")
    except requests.exceptions.RequestException as e:
        print(f"❌ Exception occurred: {e}\n")
