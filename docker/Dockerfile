# providing a gocd agent with private repo support.
# provide the ssh private key in /gocd/github.pem and
# make sure to have github grant access by uploading
# the ssh public key before.

FROM gocd/gocd-agent:16.12.0

COPY wait_for_go_agent.sh /usr/bin
# COPY make_port_configurable.sh .
COPY known_hosts /etc/ssh/github_known_hosts
COPY ssh_config  /etc/ssh
COPY bh_init     /gocd/

COPY install_awscli.sh .
COPY install_docker.sh .
COPY install_shunit.sh .
COPY install_shellcheck.sh .

# plugins

ADD  https://github.com/ind9/gocd-s3-artifacts/releases/download/v2.0.2/s3fetch-assembly-2.0.2.jar   /plugins
ADD  https://github.com/ind9/gocd-s3-artifacts/releases/download/v2.0.2/s3publish-assembly-2.0.2.jar /plugins

RUN  /bin/bash install_awscli.sh && \
     /bin/bash install_docker.sh && \
     /bin/bash install_shunit.sh && \
     /bin/bash install_shellcheck.sh

RUN  chown -R go.go /gocd /plugins /etc/ssh/github_known_hosts && \
     chmod 0700 /gocd/bh_init

# RUN  /bin/bash make_port_configurable.sh

CMD /gocd/bh_init
