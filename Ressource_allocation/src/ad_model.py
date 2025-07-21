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

import pandas as pd
import numpy as np
import tensorflow as tf
from sklearn.preprocessing import StandardScaler
from sklearn.metrics import accuracy_score, f1_score, precision_score, recall_score

# Charger le modèle entraîné
model = tf.keras.models.load_model("anomaly_detection_model.h5")

# Charger les données de validation
attack_data_df = pd.read_csv("synthetic_attack_data.csv")

# Encodage des labels
attack_data_df['attack_type'] = attack_data_df['attack_type'].map({'Normal': 0, 'DDoS_UDP': 1, 'Bandwidth_Hog': 2})

# Sélection des caractéristiques
features = ['dl_n_samples', 'dl_buffer_bytes', 'tx_brate_downlink_mbps', 'tx_pkts_downlink',
            'ul_n_samples', 'ul_buffer_bytes', 'rx_brate_uplink_mbps', 'rx_pkts_uplink']
X_val = attack_data_df[features]
y_val = attack_data_df['attack_type']

# Normalisation des données
scaler = StandardScaler()
X_scaled = scaler.fit_transform(X_val)

# Reshape pour LSTM (échantillons, timesteps, caractéristiques)
X_val = X_scaled.reshape((X_scaled.shape[0], 1, X_scaled.shape[1]))

# Prédictions
y_pred = np.argmax(model.predict(X_val), axis=1)

# Calcul des métriques de validation
accuracy = accuracy_score(y_val, y_pred)
precision = precision_score(y_val, y_pred, average='weighted')
recall = recall_score(y_val, y_pred, average='weighted')
f1 = f1_score(y_val, y_pred, average='weighted')

# Affichage des résultats
print(f"Exactitude: {accuracy:.4f}")
print(f"Précision: {precision:.4f}")
print(f"Rappel: {recall:.4f}")
print(f"Score F1: {f1:.4f}")
