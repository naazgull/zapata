FROM ubuntu:14.04
MAINTAINER Pedro (n@zgul) Figueiredo <pedro.figueiredo@gmail.com>

# install zapata from Launchpad PPA
RUN apt-get -qq update
RUN apt-get -qqy upgrade
RUN apt-get -qqy dist-upgrade
RUN echo "deb http://ppa.launchpad.net/dfz-team/zapata/ubuntu trusty main" > /etc/apt/sources.list.d/dfz-team-zapata-trusty.list
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv-keys FEDBD267
RUN apt-get -qq update
RUN apt-get -qqy install nginx sendmail zapata-rest-server mongodb-clients mongodb-dev
RUN mkdir -p /etc/zapata/backend-available
RUN mkdir -p /etc/zapata/backend-enabled
RUN mkdir -p /etc/zapata/conf.d

# add start script
COPY sbin/zapata-docker-start /sbin/

# add configuration files > in version 0.1.6 it will be removed
# since this files are included in the PPA package
COPY etc/init.d/zapata-rest-server /etc/init.d/
COPY etc/zapata/backend-available/ /etc/zapata/backend-available/
COPY etc/zapata/backend-enabled/ /etc/zapata/backend-enabled/
COPY etc/zapata/conf.d/ /etc/zapata/conf.d/
COPY etc/zapata/zapata.conf /etc/zapata/zapata.conf

# expose zapata listening ports
EXPOSE 80 9090

# launch the zapata rest daemon
CMD [ "/sbin/zapata-docker-start" ]
