FROM debian:buster

RUN apt-get update && \
	apt-get install --no-install-recommends -y \
	make \
	clang \
	python3 \
	python3-pip \
	php-cgi \
	procps

RUN python3 -m pip install chardet requests_toolbelt
RUN mkdir /webserv
COPY . /webserv/

CMD ["bash", "/webserv/Docker/StartWebserv.sh"]
