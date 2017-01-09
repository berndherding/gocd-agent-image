#curl -fsSL https://test.docker.com/ | sed -e 's/\(apt-get install.*docker-engine\)/RUNLEVEL=1 \1/' | /bin/bash

apt-get install -y docker-engine=1.11.2~rc1-0~trusty
