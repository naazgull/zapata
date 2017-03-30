FROM naazgull/zapata:xenial-base

WORKDIR /root

RUN wget -O - https://repo.dfz.pt/apt/dfz_apt.key | apt-key add -
RUN echo "deb https://repo.dfz.pt/apt/ubuntu $(lsb_release -sc) main" | tee /etc/apt/sources.list.d/naazgull.list
RUN apt-get update && apt-get install -y 'zapata-*'
RUN sed -i 's@#pragma message@//#pragma message@g' /usr/include/zmq_utils.h

VOLUME /var/log/zapata