FROM naazgull/zapata:yakkety-base

WORKDIR /root

RUN wget -O - https://repo.dfz.pt/apt/dfz_apt.key | apt-key add -
RUN echo "deb https://repo.dfz.pt/apt/ubuntu $(lsb_release -sc) main" | tee /etc/apt/sources.list.d/naazgull.list
RUN apt-get update && apt-get install -y zapata-base zapata-json zapata-http zapata-events zapata-zmq zapata-rest zapata-gen zapata-postgresql zapata-mariadb zapata-mongodb zapata-redis zapata-mqtt

VOLUME /var/log/zapata