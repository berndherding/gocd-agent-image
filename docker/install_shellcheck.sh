apt-get update
apt-get install -y libgmp-dev
curl -o /usr/bin/shellcheck https://s3.eu-central-1.amazonaws.com/bherding-gocd-agent-image/shellcheck-0.4.4
chmod 755 /usr/bin/shellcheck
