
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

# Compilation du projet principal
WORKDIR /opt/e2sim/build
RUN cmake .. && make package \
 && cmake .. -DDEV_PKG=1 && make package \
 && dpkg -i e2sim_1.0.0_amd64.deb e2sim-dev_1.0.0_amd64.deb \
 && rm -f *.deb

# Compilation du module KPM
WORKDIR /opt/e2sim/kpm_e2sm/.build
RUN cmake .. && make install
