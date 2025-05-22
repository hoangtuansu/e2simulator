
# Étape 1 : Image de base
FROM debian:trixie-slim

# Étape 2 : Installer les outils nécessaires
RUN apt update && \
    apt install -y build-essential git cmake autoconf wget bison flex libtool

# Étape 3 : Cloner et installer asn1c
RUN git clone https://github.com/mouse07410/asn1c.git /opt/asn1c
WORKDIR /opt/asn1c
RUN autoreconf -iv && ./configure && make -j$(nproc) && make install

# Étape 4 : Revenir dans le répertoire de travail
WORKDIR /e2sim

# Étape 5 : Copier les fichiers ASN.1
COPY asn1_files/ ./asn1_files/

# Étape 6 : Copier le main.c de test
COPY main.c .

# Étape 7 : Créer le dossier pour les fichiers générés
RUN mkdir -p asn1_generated

# Étape 8 : Générer le code source ASN.1
RUN asn1c -fcompound-names -pdu=all -D asn1_generated asn1_files/*.asn

# Étape 9 : Compiler le main de test
RUN gcc -o /usr/local/bin/kpm_sim main.c $(find asn1_generated -name '*.c' ! -name 'converter-example.c') -Iasn1_generated -lm

# Étape 10 : Exécuter le programme automatiquement
CMD ["/usr/local/bin/kpm_sim", "10.180.113.156", "32222"]
