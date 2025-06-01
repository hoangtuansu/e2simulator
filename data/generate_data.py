# ==================================================================================
#  Copyright (c) 2020 HCL Technologies Limited.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
# ==================================================================================

import numpy as np
import pandas as pd
from minio import Minio
from minio.error import S3Error
import urllib3
urllib3.disable_warnings()

# Définition du nombre d'échantillons
num_samples = 16000

# Définition des types de trafic
traffic_types = ['eMBB', 'mMTC', 'URLLC']
attack_types = ['Normal', 'DDoS_UDP', 'Bandwidth_Hog']

# Génération des données simulées
np.random.seed(42)
data = {
    'traffic_type': np.random.choice(traffic_types, num_samples),
    'attack_type': np.random.choice(attack_types, num_samples, p=[0.7, 0.2, 0.1]),
    'dl_n_samples': np.random.randint(50, 500, num_samples),
    'dl_buffer_bytes': np.random.randint(1000, 50000, num_samples),
    'tx_brate_downlink_mbps': np.random.uniform(1, 100, num_samples),
    'tx_pkts_downlink': np.random.randint(10, 1000, num_samples),
    'ul_n_samples': np.random.randint(50, 500, num_samples),
    'ul_buffer_bytes': np.random.randint(1000, 50000, num_samples),
    'rx_brate_uplink_mbps': np.random.uniform(1, 100, num_samples),
    'rx_pkts_uplink': np.random.randint(10, 1000, num_samples)
}

# Ajustement des valeurs en fonction du type d'attaque
for i in range(num_samples):
    if data['attack_type'][i] == 'DDoS_UDP':
        data['rx_brate_uplink_mbps'][i] = np.random.uniform(80, 150)  # Très haut débit en uplink
        data['rx_pkts_uplink'][i] = np.random.randint(800, 2000)  # Forte fréquence de paquets
    elif data['attack_type'][i] == 'Bandwidth_Hog':
        data['tx_brate_downlink_mbps'][i] = np.random.uniform(50, 200)  # Saturation de la bande passante
        data['tx_pkts_downlink'][i] = np.random.randint(500, 2000)  # Beaucoup de paquets envoyés

# Création d'un DataFrame
attack_data_df = pd.DataFrame(data)

# Sauvegarde des données générées
csv_file = "synthetic_attack_data.csv"
attack_data_df.to_csv(csv_file, index=False)

print("Dataset généré et sauvegardé sous 'synthetic_attack_data.csv'")

# Initialisation du client Minio
client = Minio(
    "10.180.113.156:31119",
    access_key="3dLD4Oke2Tm6u2knHaKk",
    secret_key="JCnrxQVbLGPNzMgCoe8aaZXLjnymLy4kO2maAL1e",
    secure=True, #https activé
    http_client=urllib3.PoolManager(cert_reqs='CERT_NONE')  # Ignore la vérification SSL
)

# Nom du bucket S3
bucket_name = "e2data"
object_name = "synthetic_attack_data.csv"

# Vérification de l'existence du bucket
found = client.bucket_exists(bucket_name)
if not found:
    client.make_bucket(bucket_name)
    print(f"Bucket '{bucket_name}' créé.")
else:
    print(f"Bucket '{bucket_name}' déjà existant.")

# Envoi du fichier vers S3
try:
    client.fput_object(bucket_name, object_name, csv_file)
    print(f"Fichier '{csv_file}' envoyé à S3 dans le bucket '{bucket_name}'.")
except S3Error as e:
    print(f"Erreur lors de l'envoi du fichier à S3: {e}")
