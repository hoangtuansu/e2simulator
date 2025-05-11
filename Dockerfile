FROM debian:trixie-slim

# Installer les dépendances
RUN apt update && \
    apt install -y build-essential git cmake autoconf wget bison flex libtool

# Cloner asn1c dans /opt
RUN git clone https://github.com/mouse07410/asn1c.git /opt/asn1c

# Compiler asn1c
WORKDIR /opt/asn1c
RUN autoreconf -iv && ./configure && make -j$(nproc) && make install

# Copier TES fichiers ASN.1 dans l'image
WORKDIR /asn1
COPY asn1_files/ ./asn1_files/

# Créer le dossier de sortie
RUN mkdir -p asn1_generated

# Compiler tes fichiers ASN.1
RUN asn1c -pdu=auto -fincludes-quoted -fcompound-names -findirect-choice -fno-include-deps -no-gen-example -no-gen-OER -D asn1_generated asn1_files/*.asn

# Spécifie le point d’entrée (optionnel)
# CMD ["bash"]
