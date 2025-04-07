FROM debian:bullseye-slim

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    autoconf \
    automake \
    bison \
    build-essential \
    cmake \
    flex \
    git \
    libboost-all-dev \
    libsctp-dev \
    libtool \
    lksctp-tools \
    net-tools \
    && rm -rf /var/lib/apt/lists/*

# Préparer les répertoires nécessaires
RUN mkdir -p /opt/e2sim/asn1c \
    /opt/e2sim/src \
    /usr/local/include/nlohmann \
    /opt/e2sim/build \
    /opt/e2sim/kpm_e2sm/asn1c \
    /opt/e2sim/kpm_e2sm/.build

# Copier tous les fichiers avant compilation
COPY src/nlohmann_json.hpp /usr/local/include/nlohmann/json.hpp
COPY CMakeLists.txt /opt/e2sim/
COPY asn1c/ /opt/e2sim/asn1c/
COPY src/ /opt/e2sim/src/
COPY kpm_e2sm/ /opt/e2sim/kpm_e2sm/

# Compilation du projet principal (e2sim)
WORKDIR /opt/e2sim/build
RUN cmake .. && make -j$(nproc) && make package \
 && cp e2sim_*.deb /tmp/

# Nettoyage entre les deux builds pour éviter conflits + reconfiguration
RUN rm -rf * && cmake .. -DDEV_PKG=1 && make -j$(nproc) && make package \
 && cp /tmp/e2sim_*.deb ./

# Installation des paquets générés sans conflit
RUN dpkg -i e2sim_1.0.0_amd64.deb && \
    dpkg -i e2sim-dev_1.0.0_amd64.deb || true && \
    rm -f *.deb

# ✅ Solution 2: Copier les headers ASN.1 pour KPM (si nécessaires)
RUN cp /opt/e2sim/build/asn1c/*.h /opt/e2sim/kpm_e2sm/asn1c/ || true

# Compilation du module KPM (sans make install)
WORKDIR /opt/e2sim/kpm_e2sm/.build
RUN cmake .. && make -j$(nproc)

# Lancer automatiquement e2sim_simulator
CMD ["/usr/local/bin/e2sim_simulator"]