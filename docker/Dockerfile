# providing a gocd agent with private repo support.
# provide the ssh private key in /gocd/github.pem and
# make sure to have github grant access by uploading
# the ssh public key before.

FROM gocd/gocd-agent:16.12.0

COPY known_hosts          /etc/ssh/github_known_hosts
COPY ssh_config           /etc/ssh

COPY wait_for_go_agent.sh /usr/bin

RUN  chown go.go /etc/ssh/github_known_hosts

COPY install_awscli.sh .
COPY install_docker.sh .
COPY install_shunit.sh .
COPY install_shellcheck.sh .

RUN  /bin/bash install_awscli.sh
RUN  /bin/bash install_docker.sh
RUN  /bin/bash install_shunit.sh
RUN  /bin/bash install_shellcheck.sh

RUN  mkdir /gocd
RUN  chown go.go /gocd

COPY bh_init     /gocd/bh_init
RUN  chown go.go /gocd/bh_init
RUN  chmod 0700  /gocd/bh_init

# plugins

RUN  mkdir /gocd-plugins
ADD  https://github.com/ind9/gocd-s3-artifacts/releases/download/v2.0.2/s3fetch-assembly-2.0.2.jar   /gocd-plugins
ADD  https://github.com/ind9/gocd-s3-artifacts/releases/download/v2.0.2/s3publish-assembly-2.0.2.jar /gocd-plugins

CMD /gocd/bh_init
