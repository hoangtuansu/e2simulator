# Étape 1 : Image de base
FROM debian:trixie-slim

# Étape 2 : Installer les dépendances système et les certificats SSL
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf \
    automake \
    bison \
    build-essential \
    ca-certificates \
    cmake \
    flex \
    git \
    libboost-all-dev \
    libsctp-dev \
    libtool \
    lksctp-tools \
    net-tools \
    wget \
    && rm -rf /var/lib/apt/lists/*

# Étape 3 : Installer ASN1C depuis GitHub
RUN git clone https://github.com/mouse07410/asn1c.git /opt/asn1c && \
    cd /opt/asn1c && \
    autoreconf -iv && \
    ./configure && \
    make -j$(nproc) && \
    make install

# Étape 4 : Préparer les dossiers de e2sim
RUN mkdir -p /opt/e2sim/asn1c \
    /opt/e2sim/src \
    /usr/local/include/nlohmann \
    /opt/e2sim/build

# Étape 5 : Copier les fichiers de configuration et sources
COPY src/nlohmann_json.hpp /usr/local/include/nlohmann/json.hpp
COPY CMakeLists.txt /opt/e2sim/
COPY asn1c/ /opt/e2sim/asn1c
COPY src/ /opt/e2sim/src
COPY asn1_files/ /opt/e2sim/asn1_files
COPY rc_e2sm/ /opt/e2sim/rc_e2sm/

# Étape 6 : Générer le code ASN.1 automatiquement
RUN mkdir -p /opt/e2sim/asn1_generated && \
    asn1c -fcompound-names -pdu=all -D /opt/e2sim/asn1_generated /opt/e2sim/asn1_files/*.asn

# # Étape 7 : Construire e2sim
RUN rm -rf /opt/e2sim/build && mkdir -p /opt/e2sim/build
WORKDIR /opt/e2sim/build

# Première compilation (release)
RUN cmake .. && make package

# Nettoyage entre les deux builds pour éviter les conflits
RUN rm -rf * && cmake .. -DDEV_PKG=1 && make package

# Installation des paquets générés
RUN dpkg -i e2sim_1.0.0_amd64.deb e2sim-dev_1.0.0_amd64.deb && rm -f *.deb

# Étape 8 : Construction du protocole KPM (si nécessaire)
RUN mkdir -p /opt/e2sim/kpm_e2sm/asn1c /opt/e2sim/kpm_e2sm/.build
COPY ./kpm_e2sm/ /opt/e2sim/kpm_e2sm/
COPY ./kpm_e2sm/src/kpm/config.json /opt/e2sim/kpm_e2sm/
WORKDIR /opt/e2sim/kpm_e2sm/.build
RUN cmake .. && make install
