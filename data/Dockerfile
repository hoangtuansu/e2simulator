# Utiliser une image Python légère comme base
FROM python:3.10-slim

# Définir le répertoire de travail dans le conteneur
WORKDIR /app

# Copier uniquement le fichier des dépendances pour optimiser le cache Docker
COPY requirements.txt .

# Installer les dépendances
RUN pip install --no-cache-dir -r requirements.txt

# Copier le script Python dans le conteneur
COPY src/generate_data.py .

# Définir la commande d'exécution du conteneur
CMD ["python", "generate_data.py"]

