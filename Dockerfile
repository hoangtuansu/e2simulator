FROM oran1.ens.ad.etsmtl.ca:5000/oran/asn1c:latest AS builder
FROM debian:trixie-slim as run

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

RUN mkdir -p /usr/local/include/nlohmann \
	/usr/local/include/strasser \
	/opt/e2sim/build


COPY . /opt/e2sim/

RUN cp /opt/e2sim/src/nlohmann_json.hpp /usr/local/include/nlohmann/json.hpp && \
	cp /opt/e2sim/src/csv.h /usr/local/include/strasser/csv.h 

COPY --from=builder /asn1/asn1_generated/* /opt/e2sim/asn1c/

WORKDIR /opt/e2sim/build

RUN cmake .. && make package \
&& cmake .. -DDEV_PKG=1 && make package \
&& dpkg -i e2sim_1.0.0_amd64.deb e2sim-dev_1.0.0_amd64.deb \
&& rm -f *.deb
	
RUN mkdir -p /opt/e2sim/kpm_e2sm/asn1c /opt/e2sim/kpm_e2sm/.build

COPY ./kpm_e2sm/ /opt/e2sim/kpm_e2sm/
COPY ./kpm_e2sm/src/kpm/config.json /opt/e2sim/kpm_e2sm/

WORKDIR /opt/e2sim/kpm_e2sm/.build
RUN cmake .. && make install
