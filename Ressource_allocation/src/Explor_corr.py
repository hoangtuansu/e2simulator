
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns
import os 

# Charger les données
data_path = '~/e2sim/synthetic_metric_data.csv'  # Mets ici le chemin vers ton fichier CSV (place-le dans le dossier où tu lances ce script)
df = pd.read_csv(data_path)
df['user_outage'] = np.where(df['radio_resource_utilization_percent'] > 80, 1, 0)

# Créer dossier pour sauvegarde
os.makedirs('plots', exist_ok=True)

# Afficher un aperçu
print(df.head())

# Heatmap de corrélation
plt.figure(figsize=(12, 
10))
sns.heatmap(df.corr(), annot=True, fmt=".2f", cmap="coolwarm", linewidths=0.5)
plt.title("Matrice de corrélation des variables")
plt.tight_layout()
plt.savefig('plots/correlation_heatmap.png')
plt.close()

# Scatter plots pour available_prbs_dl
main_features = ['throughput_dl_mbps', 'throughput_ul_mbps', 'radio_resource_utilization_percent', 'num_active_ues']
for feature in main_features:
    plt.figure(figsize=(6, 4))
    sns.scatterplot(data=df, x=feature, y='available_prbs_dl')
    plt.title(f"{feature} vs available_prbs_dl")
    plt.tight_layout()
    plt.savefig(f'plots/{feature}_vs_available_prbs_dl.png')
    plt.close()

# Scatter plots pour available_prbs_ul
for feature in main_features:
    plt.figure(figsize=(6, 4))
    sns.scatterplot(data=df, x=feature, y='available_prbs_ul')
    plt.title(f"{feature} vs available_prbs_ul")
    plt.tight_layout()
    plt.savefig(f'plots/{feature}_vs_available_prbs_ul.png')
    plt.close()

# Scatter plots pour user_outage
for feature in main_features:
    plt.figure(figsize=(6, 4))
    sns.scatterplot(data=df, x=feature, y='user_outage')
    plt.title(f"{feature} vs user_outage")
    plt.tight_layout()
    plt.savefig(f'plots/{feature}_vs_user_outage.png')
    plt.close()

print("✅ Graphiques sauvegardés dans le dossier 'plots'")
