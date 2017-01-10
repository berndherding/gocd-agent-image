# curl -fsSL https://test.docker.com/ | sed -e 's/\(apt-get install.*docker-engine\)/RUNLEVEL=1 \1/' | /bin/bash

cat <<EOF > /etc/apt/sources.list.d/docker.list
deb https://apt.dockerproject.org/repo ubuntu-trusty main
EOF

apt-key adv \
  --keyserver hkp://ha.pool.sks-keyservers.net:80 \
  --recv-keys 58118E89F3A912897C070ADBF76221572C52609D

apt-get update

# apt-cache showpkg docker-engine

apt-get install -y docker-engine=1.11.2-0~trusty
