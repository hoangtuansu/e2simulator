import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import warnings
from sklearn.ensemble import RandomForestRegressor, RandomForestClassifier
from sklearn.metrics import (mean_squared_error, mean_absolute_error, r2_score,
                             confusion_matrix, classification_report, accuracy_score, f1_score)
from sklearn.model_selection import train_test_split, GridSearchCV, cross_val_score
import time

warnings.filterwarnings('ignore')

# Chargement des données
file_path = '~/e2sim/synthetic_metric_data.csv'
data_df = pd.read_csv(file_path)

# Création de la métrique User Outage (utilisation >80% → risque d'outage)
data_df['user_outage'] = np.where(data_df['radio_resource_utilization_percent'] > 80, 1, 0)

# Analyse des distributions (histogrammes)
data_df.hist(bins=30, figsize=(12, 8))
plt.tight_layout()
plt.savefig('feature_distributions.png')
plt.close()
print("✅ Histogrammes des distributions sauvegardés dans feature_distributions.png")

# Transformation log des throughput
data_df['throughput_dl_mbps_log'] = np.log1p(data_df['throughput_dl_mbps'])
data_df['throughput_ul_mbps_log'] = np.log1p(data_df['throughput_ul_mbps'])

# Nouvelles variables
data_df['throughput_dl_per_user'] = data_df['throughput_dl_mbps'] / (data_df['num_active_ues'] + 1)
data_df['throughput_ul_per_user'] = data_df['throughput_ul_mbps'] / (data_df['num_active_ues'] + 1)
data_df['utilization_x_throughput'] = data_df['radio_resource_utilization_percent'] * data_df['throughput_dl_mbps']

# Features communes
features = ['throughput_dl_mbps_log', 'throughput_ul_mbps_log',
            'radio_resource_utilization_percent', 'num_active_ues',
            'throughput_dl_per_user', 'throughput_ul_per_user', 'utilization_x_throughput']

# Séparation X et y
X = data_df[features]
y_reg = data_df[['available_prbs_dl', 'available_prbs_ul']]
y_class = data_df['user_outage']

# Division train/test
X_train, X_test, y_reg_train, y_reg_test, y_class_train, y_class_test = train_test_split(
    X, y_reg, y_class, test_size=0.3, random_state=42
)

# ✅ Optimisation régression
param_grid = {'n_estimators': [50, 100], 'max_depth': [5, 10, None], 'min_samples_split': [2, 5]}
regressor = GridSearchCV(RandomForestRegressor(random_state=42), param_grid, cv=3, n_jobs=-1, verbose=0)

start_time = time.time()
regressor.fit(X_train, y_reg_train)
training_time_reg = time.time() - start_time

# ✅ Cross-validation régression
scores_r2_dl = cross_val_score(regressor.best_estimator_, X_train, y_reg_train['available_prbs_dl'], cv=3, scoring='r2')
print(f"✅ R² cross-validation (PRBs DL) : moyenne = {scores_r2_dl.mean():.2f}, std = {scores_r2_dl.std():.2f}")

# Prédictions régression
y_reg_pred = regressor.predict(X_test)
prbs_dl_pred = y_reg_pred[:, 0]
prbs_ul_pred = y_reg_pred[:, 1]

# Évaluation régression
mse_dl = mean_squared_error(y_reg_test['available_prbs_dl'], prbs_dl_pred)
mae_dl = mean_absolute_error(y_reg_test['available_prbs_dl'], prbs_dl_pred)
r2_dl = r2_score(y_reg_test['available_prbs_dl'], prbs_dl_pred)

mse_ul = mean_squared_error(y_reg_test['available_prbs_ul'], prbs_ul_pred)
mae_ul = mean_absolute_error(y_reg_test['available_prbs_ul'], prbs_ul_pred)
r2_ul = r2_score(y_reg_test['available_prbs_ul'], prbs_ul_pred)

# ✅ Modèle classification spécifique user_outage
classifier = RandomForestClassifier(random_state=42, class_weight='balanced', n_estimators=100, max_depth=10)
classifier.fit(X_train, y_class_train)

# Cross-validation classification
scores_acc = cross_val_score(classifier, X_train, y_class_train, cv=3, scoring='accuracy')
scores_f1 = cross_val_score(classifier, X_train, y_class_train, cv=3, scoring='f1')
print(f"✅ Cross-validation User Outage : Accuracy moy = {scores_acc.mean():.2f}, F1 moy = {scores_f1.mean():.2f}")

# Prédictions classification
y_class_pred = classifier.predict(X_test)

# Évaluation classification
accuracy_outage = accuracy_score(y_class_test, y_class_pred)
f1_outage = f1_score(y_class_test, y_class_pred)
conf_matrix = confusion_matrix(y_class_test, y_class_pred)
class_report = classification_report(y_class_test, y_class_pred, digits=2, output_dict=True)

print(f"=== Évaluation PRBs DL ===\nMSE: {mse_dl:.2f}, MAE: {mae_dl:.2f}, R²: {r2_dl:.2f}")
print(f"=== Évaluation PRBs UL ===\nMSE: {mse_ul:.2f}, MAE: {mae_ul:.2f}, R²: {r2_ul:.2f}")
print(f"=== User Outage Classification ===\nAccuracy: {accuracy_outage:.2%}, F1: {f1_outage:.2f}")
print(f"Confusion Matrix:\n{conf_matrix}")
print(f"Training Time (Reg): {training_time_reg:.2f} sec")

# Sauvegarde Excel
results_df = pd.DataFrame({
    'Metric': ['MSE_DL', 'MAE_DL', 'R2_DL', 'MSE_UL', 'MAE_UL', 'R2_UL', 'Accuracy_Outage', 'F1_Outage', 'Training_Time_reg'],
    'Value': [mse_dl, mae_dl, r2_dl, mse_ul, mae_ul, r2_ul, accuracy_outage, f1_outage, training_time_reg]
})
conf_matrix_df = pd.DataFrame(conf_matrix, columns=['Pred_0', 'Pred_1'], index=['True_0', 'True_1'])
class_report_df = pd.DataFrame(class_report).transpose()

with pd.ExcelWriter('model_evaluation.xlsx') as writer:
    results_df.to_excel(writer, sheet_name='Summary', index=False)
    conf_matrix_df.to_excel(writer, sheet_name='Confusion_Matrix')
    class_report_df.to_excel(writer, sheet_name='Classification_Report')

# Graphiques : valeurs réelles vs prédites (DL et UL)
fig, axs = plt.subplots(1, 2, figsize=(12, 6))
axs[0].scatter(y_reg_test['available_prbs_dl'], prbs_dl_pred, alpha=0.6)
axs[0].plot([y_reg_test['available_prbs_dl'].min(), y_reg_test['available_prbs_dl'].max()],
            [y_reg_test['available_prbs_dl'].min(), y_reg_test['available_prbs_dl'].max()], 'r--')
axs[0].set_xlabel("PRBs DL Réels")
axs[0].set_ylabel("PRBs DL Prédits")
axs[0].set_title("PRBs DL : Réel vs Prédit")

axs[1].scatter(y_reg_test['available_prbs_ul'], prbs_ul_pred, alpha=0.6)
axs[1].plot([y_reg_test['available_prbs_ul'].min(), y_reg_test['available_prbs_ul'].max()],
            [y_reg_test['available_prbs_ul'].min(), y_reg_test['available_prbs_ul'].max()], 'r--')
axs[1].set_xlabel("PRBs UL Réels")
axs[1].set_ylabel("PRBs UL Prédits")
axs[1].set_title("PRBs UL : Réel vs Prédit")

plt.tight_layout()
plt.savefig('real_vs_predicted_prbs.png')
plt.close()

# Détails des prédictions
predictions_df = pd.DataFrame({
    'available_prbs_dl_true': y_reg_test['available_prbs_dl'].values,
    'available_prbs_dl_pred': prbs_dl_pred,
    'available_prbs_ul_true': y_reg_test['available_prbs_ul'].values,
    'available_prbs_ul_pred': prbs_ul_pred,
    'user_outage_true': y_class_test.values,
    'user_outage_pred': y_class_pred
})
predictions_df.to_excel('detailed_predictions.xlsx', index=False)
print('✅ detailed_predictions.xlsx exporté avec succès.')
