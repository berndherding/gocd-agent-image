#   awscli

curl https://s3.amazonaws.com/aws-cli/awscli-bundle.zip -o awscli-bundle.zip
unzip awscli-bundle.zip
./awscli-bundle/install -i /usr/local/aws -b /usr/local/bin/aws



#   docker

cat <<EOF > /etc/apt/sources.list.d/docker.list
deb https://apt.dockerproject.org/repo ubuntu-trusty main
EOF

apt-key adv \
  --keyserver hkp://ha.pool.sks-keyservers.net:80 \
  --recv-keys 58118E89F3A912897C070ADBF76221572C52609D

apt-get update
# apt-cache showpkg docker-engine
apt-get install -y docker-engine=1.11.2-0~trusty



#   shunit

git clone https://github.com/kward/shunit2.git
cp shunit2/source/2.1/src/shunit2 /usr/local/bin/shunit



#   shellcheck

#apt-get update
apt-get install -y libgmp-dev
curl -o /usr/bin/shellcheck https://s3.eu-central-1.amazonaws.com/bherding-gocd-agent-image/shellcheck-0.4.4
chmod 755 /usr/bin/shellcheck



#   htpasswd
apt-get install -y apache2-utils



#   jq
apt-get install -y jq
