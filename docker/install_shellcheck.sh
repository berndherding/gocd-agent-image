#cat <<EOF >> /etc/apt/sources.list
#deb http://de.archive.ubuntu.com/ubuntu trusty-backports universe
#EOF
#apt-get update
#apt-get install -y shellcheck

curl -o /usr/bin/shellcheck https://s3.eu-central-1.amazonaws.com/bherding-gocd-agent-image/shellcheck-0.4.4
