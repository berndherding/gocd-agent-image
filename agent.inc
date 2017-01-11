#!/bin/bash

shopt -s extglob

[ ! -e "$(dirname "${BASH_SOURCE[0]}")/inc"    ] && ln -s ../gocd-base-ami/inc    "$(dirname "${BASH_SOURCE[0]}")/inc"
[ ! -e "$(dirname "${BASH_SOURCE[0]}")/target" ] && ln -s ../gocd-base-ami/target "$(dirname "${BASH_SOURCE[0]}")/target"

# shellcheck source=/dev/null
. "$(dirname "${BASH_SOURCE[0]}")/inc/commons.inc"

# shellcheck source=/dev/null
. "$(dirname "${BASH_SOURCE[0]}")/inc/keys.inc"

ERROR_build_dockerBuild=1
ERROR_build_dockerRun=2
ERROR_build_dockerCommit=3
ERROR_build_dockerExec_movePlugins=4
ERROR_build_dockerExec_removeGuid=5
ERROR_build_dockerRemove_container=6
ERROR_destroy_dockerRemove_container=7
ERROR_run_dockerExec_waitForGoAgent=8
ERROR_run_dockerRun=9
ERROR_ship_dockerPush=10



#
# TODO: check for docker, e.g. by calling docker version
#



function getImageTag() {
  local imageName=${1:-gocd-agent}

  imageTag=$imageName:${GO_PIPELINE_COUNTER:-0}

  if [ "$imageTag" != "$imageName:0" ] ; then

    gitHash=${GO_REVISION_GOCD_AGENT_IMAGE:0:7}
    gitHash=${gitHash:-$(git rev-parse --short HEAD)}

    imageTag="$imageTag-$gitHash"
  fi

  echo "$imageTag"
}



function build() {
  local imageName=$1

  containerName=${imageName//:/-}
  imageName="$(getDockerHubUsername)/$imageName"
  goServerUrl="$(cat "$(dirname "${BASH_SOURCE[0]}")"/target/goServerPublicIpAddress)"

  docker build --no-cache -t "$imageName" "$(dirname "${BASH_SOURCE[0]}")"/docker | tee /tmp/agent-build.out \
  || return $ERROR_build_dockerBuild

  # why run at all? -> the gocd agent extracts itself on first run
  containerId=$(docker run --name "$containerName" -d \
    -e GO_SERVER_URL="$goServerUrl:80" \
    "$imageName"
  ) || return $ERROR_build_dockerRun

  containerId=${containerId:0:12}

  docker exec -t "$containerName" /bin/bash /usr/bin/wait_for_go_agent.sh 120 \
  # || return $ERROR_build_dockerExec_waitForGoAgent

  docker exec -t "$containerName" /bin/bash -c "
    mv /gocd-plugins/* /var/lib/go-agent/plugins/external
    chown go.go /var/lib/go-agent/plugins/external/*
  " || return $ERROR_build_dockerExec_movePlugins 

  docker commit "$containerId" "$imageName" || return $ERROR_build_dockerCommit

  docker rm -f "$containerName" || return $ERROR_build_dockerRemove_container

  containerId=$(docker run --name "$containerName" -d \
    -e GO_SERVER_URL="$goServerUrl:80" \
    "$imageName" \
    sleep 300
  ) || return $ERROR_build_dockerRun

  docker exec -t "$containerName" /bin/bash -c "
    rm /var/lib/go-agent/config/guid.txt
  " || return $ERROR_build_dockerExec_removeGuid

  docker commit --change='CMD /gocd/bh_init' "$containerId" "$imageName" \
  || return $ERROR_build_dockerCommit

  docker rm -f "$containerName" || return $ERROR_build_dockerRemove_container

  return 0
}



function ship() {
  local imageName=$1

  docker login --username "$(getDockerHubUsername)" --password "$(getDockerHubPassword)"

  docker push "$(getDockerHubUsername)/$imageName" || return $ERROR_ship_dockerPush

  echo "$(getDockerHubUsername)/$imageName" > "$(dirname "${BASH_SOURCE[0]}")"/target/gocdAgentImageName

  docker logout
}



function run() {
  local imageName=$1

  containerName=${imageName//:/-}

  containerId=$(docker run --name "$containerName" -d \
    "$(getDockerHubUsername)/$imageName"
  ) || return $ERROR_run_dockerRun

  containerId=${containerId:0:12}

  docker exec -t "$containerName" /bin/bash /usr/bin/wait_for_go_agent.sh 120 \
  || return $ERROR_run_dockerExec_waitForGoAgent
}



function destroy() {
  local imageName=$1

  containerName=${imageName//:/-}

  docker rm -f "$containerName" \
  || return $ERROR_destroy_dockerRemove_container
}
