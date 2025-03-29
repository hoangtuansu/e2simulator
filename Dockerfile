FROM debian:bullseye

# Installer les dépendances nécessaires
RUN apt-get update && apt-get install -y \
  build-essential git cmake libsctp-dev lksctp-tools autoconf automake \
  libtool bison flex libboost-all-dev iputils-ping \
  net-tools nano vim tcpdump nmap \
  && apt-get clean

# Cloner la bibliothèque JSON (azadkuh)
RUN git clone https://github.com/azadkuh/nlohmann_json_release.git
RUN mkdir -p /usr/local/include/nlohmann && \
    cp nlohmann_json_release/json.hpp /usr/local/include/nlohmann

# Créer les répertoires principaux du projet
RUN mkdir -p /opt/e2sim/asn1c /opt/e2sim/src /opt/e2sim/kpm_e2sm/src/kpm /opt/e2sim/kpm_e2sm/asn1c

# Copier les fichiers principaux
COPY CMakeLists.txt /opt/e2sim/
COPY asn1c/ /opt/e2sim/asn1c/
COPY src/ /opt/e2sim/src/

# Copier les fichiers personnalisés pour le simulateur KPM
COPY kpm_e2sm/src/kpm/ /opt/e2sim/kpm_e2sm/src/kpm/
COPY kpm_e2sm/kpi_traces.json /opt/e2sim/kpm_e2sm/kpi_traces.json

# Compilation du simulateur e2sim principal
RUN mkdir /opt/e2sim/build && cd /opt/e2sim/build && \
  cmake .. && make package && cmake .. -DDEV_PKG=1 && make package

# Installer les paquets DEB générés
RUN dpkg -i /opt/e2sim/build/e2sim_1.0.0_amd64.deb \
           /opt/e2sim/build/e2sim-dev_1.0.0_amd64.deb

# Compiler le simulateur KPM spécifique
RUN mkdir -p /opt/e2sim/kpm_e2sm/.build && cd /opt/e2sim/kpm_e2sm/.build && \
  cmake .. && make install

# Script utilitaire
COPY build_and_run.sh /opt/e2sim/build_and_run.sh
RUN chmod +x /opt/e2sim/build_and_run.sh

# Commande par défaut
CMD ["/opt/e2sim/kpm_e2sm/.build/e2sim_simulator"]
