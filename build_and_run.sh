#!/bin/bash

# Nom de l'image Docker
IMAGE_NAME="e2sim_simulator"
CONTAINER_NAME="e2sim-kpm"

# Adresse du RIC (SCTP endpoint dans le cluster O-RAN)
RIC_HOST="kpm_sim service-ricplt-e2term-sctp-headless.ricplt.svc"
RIC_PORT=36422

# Construire l'image Docker
echo "🔨 Construction de l'image Docker..."
docker build -t $IMAGE_NAME .

# Supprimer un conteneur précédent s’il existe
if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}\$"; then
  echo "🧹 Suppression de l'ancien conteneur..."
  docker rm -f $CONTAINER_NAME
fi

# Lancer le conteneur avec les bons paramètres
echo "🚀 Lancement du conteneur $CONTAINER_NAME..."
docker run -d \
  --name $CONTAINER_NAME \
  --network host \
  $IMAGE_NAME \
  $RIC_HOST $RIC_PORT

# Afficher les logs en direct
echo "📡 Affichage des logs du simulateur..."
docker logs -f $CONTAINER_NAME
