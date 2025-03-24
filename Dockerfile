FROM debian:bullseye-slim
	
# Set environment variables to reduce interaction during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install all dependencies in a single RUN statement to reduce image layers
# Group related packages and sort alphabetically for better readability
# Remove package lists after installation to reduce image size
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

RUN apt-get update && apt-get install -y --no-install-recommends \
	iputils-ping \
	nano \
	nmap \
	tcpdump \
	vim \
	&& rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/e2sim/asn1c \
	/opt/e2sim/kpm_e2sm/asn1c \
	/opt/e2sim/src \
	/usr/local/include/nlohmann \
	/opt/e2sim/build \
	/opt/e2sim/kpm_e2sm/.build

RUN git clone --depth 1 https://github.com/azadkuh/nlohmann_json_release.git \
	&& cp nlohmann_json_release/json.hpp /usr/local/include/nlohmann/ \
	&& rm -rf nlohmann_json_release

COPY CMakeLists.txt /opt/e2sim/
COPY asn1c/ /opt/e2sim/asn1c
COPY src/ /opt/e2sim/src

WORKDIR /opt/e2sim/build
RUN cmake .. && make package \
&& cmake .. -DDEV_PKG=1 && make package \
&& dpkg -i e2sim_1.0.0_amd64.deb e2sim-dev_1.0.0_amd64.deb \
&& rm -f *.deb


COPY ./kpm_e2sm/ /opt/e2sim/kpm_e2sm/
COPY ./kpm_e2sm/src/kpm/config.json /opt/e2sim/kpm_e2sm/
COPY ./asn1c/ /opt/e2sim/kpm_e2sm/asn1c/

WORKDIR /opt/e2sim/kpm_e2sm/.build
RUN cmake .. && make install

#CMD kpm_sim 10.111.138.172 32222
