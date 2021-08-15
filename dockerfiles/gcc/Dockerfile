FROM ubuntu:18.04

MAINTAINER L. Jiang <l.jiang.1024@gmail.com>

ENV DEBIAN_FRONTEND=noninteractive
ADD dockerfiles/gcc/install-tools.sh /
ADD dockerfiles/gcc/Env /
RUN bash /install-tools.sh
RUN mkdir -p /var/run/sshd /var/log/supervisor
RUN sed -i 's/PermitRootLogin without-password/PermitRootLogin yes/' /etc/ssh/sshd_config
COPY authorized_keys /root/.ssh/authorized_keys
COPY supervisord.conf /etc/supervisor/conf.d/supervisord.conf

ENTRYPOINT ["/usr/bin/supervisord"]