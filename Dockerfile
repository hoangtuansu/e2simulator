FROM oran1.ens.ad.etsmtl.ca:5000/oran-sc/e2simulator:builder
	
RUN mkdir -p /opt/e2sim/kpm_e2sm/asn1c /opt/e2sim/kpm_e2sm/.build

COPY ./kpm_e2sm/ /opt/e2sim/kpm_e2sm/
COPY ./kpm_e2sm/src/kpm/config.json /opt/e2sim/kpm_e2sm/
COPY ./asn1c/ /opt/e2sim/kpm_e2sm/asn1c/

WORKDIR /opt/e2sim/kpm_e2sm/.build
RUN cmake .. && make install

#CMD kpm_sim 10.111.138.172 32222
RUN apt-get update && apt-get install -y --no-install-recommends \
	iputils-ping \
	nano \
	nmap \
	tcpdump \
	vim \
	&& rm -rf /var/lib/apt/lists/*