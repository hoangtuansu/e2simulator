import os
import numpy as np
import pandas as pd
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import mean_squared_error, mean_absolute_error, r2_score, confusion_matrix, classification_report
from sklearn.model_selection import train_test_split
from sklearn.multioutput import MultiOutputRegressor
import time
import matplotlib.pyplot as plt

# Chargement des données
file_path = '~/e2sim/synthetic_metric_data.csv'
data_df = pd.read_csv(file_path)

# Création de la métrique User Outage (utilisation >80% → risque d'outage)
data_df['user_outage'] = np.where(data_df['radio_resource_utilization_percent'] > 80, 1, 0)

# Features et cibles
X = data_df[['throughput_dl_mbps', 'throughput_ul_mbps', 'radio_resource_utilization_percent', 'num_active_ues']]
y = data_df[['available_prbs_dl', 'available_prbs_ul', 'user_outage']]

# Division train/test
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.3, random_state=42)

# Entraînement et mesure du temps
start_time = time.time()
model = MultiOutputRegressor(RandomForestRegressor(random_state=42))
model.fit(X_train, y_train)
training_time = time.time() - start_time

# Prédictions
y_pred = model.predict(X_test)

# Séparer les cibles
prbs_dl_true = y_test['available_prbs_dl']
prbs_dl_pred = y_pred[:, 0]
prbs_ul_true = y_test['available_prbs_ul']
prbs_ul_pred = y_pred[:, 1]
outage_true = y_test['user_outage']
outage_pred = np.round(y_pred[:, 2])

# Évaluation des performances
mse_dl = mean_squared_error(prbs_dl_true, prbs_dl_pred)
mae_dl = mean_absolute_error(prbs_dl_true, prbs_dl_pred)
r2_dl = r2_score(prbs_dl_true, prbs_dl_pred)

mse_ul = mean_squared_error(prbs_ul_true, prbs_ul_pred)
mae_ul = mean_absolute_error(prbs_ul_true, prbs_ul_pred)
r2_ul = r2_score(prbs_ul_true, prbs_ul_pred)

accuracy_outage = (outage_true == outage_pred).mean()
conf_matrix = confusion_matrix(outage_true, outage_pred)
class_report = classification_report(outage_true, outage_pred, digits=2, output_dict=True)

print(f"=== Évaluation PRBs DL ===\nMSE: {mse_dl:.2f}, MAE: {mae_dl:.2f}, R²: {r2_dl:.2f}")
print(f"=== Évaluation PRBs UL ===\nMSE: {mse_ul:.2f}, MAE: {mae_ul:.2f}, R²: {r2_ul:.2f}")
print(f"=== User Outage ===\nAccuracy: {accuracy_outage:.2%}\nConfusion Matrix:\n{conf_matrix}")
print(f"Training Time: {training_time:.2f} sec")

# Sauvegarde des résultats dans Excel
results_df = pd.DataFrame({
    'Metric': ['MSE_DL', 'MAE_DL', 'R2_DL', 'MSE_UL', 'MAE_UL', 'R2_UL', 'Accuracy_Outage', 'Training_Time_sec'],
    'Value': [mse_dl, mae_dl, r2_dl, mse_ul, mae_ul, r2_ul, accuracy_outage, training_time]
})

conf_matrix_df = pd.DataFrame(conf_matrix, columns=['Pred_0', 'Pred_1'], index=['True_0', 'True_1'])
class_report_df = pd.DataFrame(class_report).transpose()

with pd.ExcelWriter('model_evaluation.xlsx') as writer:
    results_df.to_excel(writer, sheet_name='Summary', index=False)
    conf_matrix_df.to_excel(writer, sheet_name='Confusion_Matrix')
    class_report_df.to_excel(writer, sheet_name='Classification_Report')

# Graphiques : valeurs réelles vs prédites (DL et UL)
fig, axs = plt.subplots(1, 2, figsize=(12, 6))
axs[0].scatter(prbs_dl_true, prbs_dl_pred, alpha=0.6)
axs[0].plot([prbs_dl_true.min(), prbs_dl_true.max()], [prbs_dl_true.min(), prbs_dl_true.max()], 'r--')
axs[0].set_xlabel("PRBs DL Réels")
axs[0].set_ylabel("PRBs DL Prédits")
axs[0].set_title("PRBs DL : Réel vs Prédit")

axs[1].scatter(prbs_ul_true, prbs_ul_pred, alpha=0.6)
axs[1].plot([prbs_ul_true.min(), prbs_ul_true.max()], [prbs_ul_true.min(), prbs_ul_true.max()], 'r--')
axs[1].set_xlabel("PRBs UL Réels")
axs[1].set_ylabel("PRBs UL Prédits")
axs[1].set_title("PRBs UL : Réel vs Prédit")

plt.tight_layout()
plt.savefig('real_vs_predicted_prbs.png')
plt.show()

print("✅ Résultats et graphiques exportés : model_evaluation.xlsx et real_vs_predicted_prbs.png")
 #Créer un DataFrame avec les vraies valeurs et les prédictions
predictions_df = pd.DataFrame({
    'available_prbs_dl_true': prbs_dl_true.values,
    'available_prbs_dl_pred': prbs_dl_pred,
    'available_prbs_ul_true': prbs_ul_true.values,
    'available_prbs_ul_pred': prbs_ul_pred,
    'user_outage_true': outage_true.values,
    'user_outage_pred': outage_pred
})

# Sauvegarder dans un fichier Excel
predictions_df.to_excel('detailed_predictions.xlsx', index=False)
print('✅ detailed_predictions.xlsx exporté avec succès.')