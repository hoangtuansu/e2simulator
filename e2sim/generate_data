import numpy as np
import pandas as pd
from minio import Minio
from minio.error import S3Error
import urllib3
urllib3.disable_warnings()

# Nombre d'échantillons à générer
num_samples = 17000  # ← modifié ici

# Types de politique d'allocation (extraits du JSON)
allocation_policies = [
    'Equal Allocation Policy',
    'Voice Priority Allocation Policy',
    'MBB Priority Allocation Policy',
    'Dedicated Resources Reservation Policy'
]

# Types de trafic simulé
ue_types = ['Voice', 'MBB']

# Initialisation des données simulées
np.random.seed(42)
data = {
    'allocation_policy': np.random.choice(allocation_policies, num_samples),
    'ue_type': np.random.choice(ue_types, num_samples),
    'user_outage_percent': np.random.uniform(2.5, 10.0, num_samples),  # en %
    'throughput_dl_mbps': np.random.uniform(0.1, 3000, num_samples),  # DL
    'throughput_ul_mbps': np.random.uniform(0.1, 1000, num_samples),  # UL
    'radio_resource_utilization_percent': np.random.uniform(10, 100, num_samples),
    'available_prbs_dl': np.random.choice([25, 50], num_samples),  # 5 MHz = 25 PRBs ; 10 MHz = 50 PRBs
    'available_prbs_ul': np.random.choice([25, 50], num_samples),
    'num_active_ues': np.random.randint(10, 100, num_samples),
    'ml_classification_accuracy': np.random.uniform(80, 95, num_samples),  # en %
    'ml_training_time_sec': np.random.uniform(5, 120, num_samples)
}

# Création du DataFrame
metric_data_df = pd.DataFrame(data)

# Sauvegarde des données
csv_file = "synthetic_metric_data.csv"
metric_data_df.to_csv(csv_file, index=False)
print("Dataset généré et sauvegardé sous 'synthetic_metric_data.csv'")

# Initialisation Minio
client = Minio(
    "10.180.113.156:31119",
    access_key="3dLD4Oke2Tm6u2knHaKk",
    secret_key="JCnrxQVbLGPNzMgCoe8aaZXLjnymLy4kO2maAL1e",
    secure=True,
    http_client=urllib3.PoolManager(cert_reqs='CERT_NONE')
)

bucket_name = "e2data"
object_name = "synthetic_metric_data.csv"

# Upload vers MinIO
try:
    if not client.bucket_exists(bucket_name):
        client.make_bucket(bucket_name)
        print(f"Bucket '{bucket_name}' créé.")
    else:
        print(f"Bucket '{bucket_name}' déjà existant.")
    
    client.fput_object(bucket_name, object_name, csv_file)
    print(f"Fichier '{csv_file}' envoyé dans le bucket S3 '{bucket_name}'.")
except S3Error as e:
    print(f"Erreur d’envoi S3 : {e}")
