FROM debian:bullseye

RUN apt-get update \
	&& apt-get install -y build-essential git cmake libsctp-dev lksctp-tools autoconf automake \
	libtool bison flex libboost-all-dev iputils-ping \
	net-tools nano vim tcpdump net-tools nmap \
  	&& apt-get clean

RUN mkdir -p /opt/e2sim/asn1c /opt/e2sim/kpm_e2sm /opt/e2sim/kpm_e2sm/asn1c /opt/e2sim/src /usr/local/include/nlohmann

RUN git clone https://github.com/azadkuh/nlohmann_json_release.git
RUN cp nlohmann_json_release/json.hpp /usr/local/include/nlohmann

COPY CMakeLists.txt /opt/e2sim/
COPY asn1c/ /opt/e2sim/asn1c
COPY src/ /opt/e2sim/src

RUN mkdir /opt/e2sim/build && cd /opt/e2sim/build \
	&& cmake .. && make package && cmake .. -DDEV_PKG=1 && make package

RUN dpkg -i /opt/e2sim/build/e2sim_1.0.0_amd64.deb /opt/e2sim/build/e2sim-dev_1.0.0_amd64.deb

COPY ./kpm_e2sm/ /opt/e2sim/kpm_e2sm
COPY ./kpm_e2sm/src/kpm/config.json /opt/e2sim/kpm_e2sm/
COPY ./asn1c/ /opt/e2sim/kpm_e2sm/asn1c

RUN mkdir /opt/e2sim/kpm_e2sm/.build && cd /opt/e2sim/kpm_e2sm/.build \
	&& cmake .. && make install

#CMD kpm_sim 10.111.138.172 32222
