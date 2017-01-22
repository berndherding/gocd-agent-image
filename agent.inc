#!/bin/bash

shopt -s extglob

[ ! -d "$(dirname "${BASH_SOURCE[0]}")/target" ] && mkdir "$(dirname "${BASH_SOURCE[0]}")/target"

ERROR_build_dockerBuild=1
ERROR_build_dockerRun=2
ERROR_build_dockerCommit=3
ERROR_build_dockerExec_removeGuid=4
ERROR_build_dockerExec_waitForGoAgent=5
ERROR_build_dockerRemove_container=6
ERROR_destroy_dockerRemove_container=7
ERROR_run_dockerExec_waitForGoAgent=8
ERROR_run_dockerRun=9
ERROR_ship_dockerPush=10
ERROR_noDockerHubUsername=11
ERROR_noDockerHubPassword=12



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
  local useBuildServer=$2

  [ -z "$DOCKER_HUB_USERNAME" ] && return $ERROR_noDockerHubUsername

  containerName=${imageName//:/-}
  imageName="$DOCKER_HUB_USERNAME/$imageName"

  if [ "use-build-server" = "$useBuildServer" ] ; then
    goServerUrl="$GO_SERVER"
  else
    goServerUrl="$(cat "$(dirname "${BASH_SOURCE[0]}")"/target/goServerPublicIpAddress)"
  fi

  docker build --no-cache -t "$imageName" "$(dirname "${BASH_SOURCE[0]}")"/docker | tee /tmp/agent-build.out \
  || return $ERROR_build_dockerBuild

  # why run at all? -> the gocd agent extracts itself on first run
  containerId=$(docker run --name "$containerName" -d \
    -e GO_SERVER="$goServerUrl" \
    "$imageName"
  ) || return $ERROR_build_dockerRun

  containerId=${containerId:0:12}

  docker exec -t "$containerName" /bin/bash /usr/bin/wait_for_go_agent.sh 180 \
  || return $ERROR_build_dockerExec_waitForGoAgent

  docker commit "$containerId" "$imageName" || return $ERROR_build_dockerCommit

# NOTE: it looks like after first start the container is writing guid.txt.
# whenever this file is deleted, the container creates it anew. somehow the
# only solution to delete guid.txt is to commit first, run anew and then
# delete the guid.txt as seen below. the image will then be committed w/o
# a guid.txt and a new one will then be created during the first startup
# after that. this way, guid.txt will be unique which is a condition for 
# an agent to auto-register with the server.

  docker rm -f "$containerName" || return $ERROR_build_dockerRemove_container

  containerId=$(docker run --name "$containerName" -d \
    -e GO_SERVER="$goServerUrl" \
    "$imageName" \
    sleep 300
  ) || return $ERROR_build_dockerRun

  docker exec -t "$containerName" /bin/bash -c "
    rm /var/lib/go-agent/config/guid.txt
    rm /var/lib/go-agent/config/agent.jks
  " || return $ERROR_build_dockerExec_removeGuid

  docker commit --change='CMD /gocd/bh_init' "$containerId" "$imageName" \
  || return $ERROR_build_dockerCommit

  docker rm -f "$containerName" || return $ERROR_build_dockerRemove_container

  return 0
}



function ship() {
  local imageName=$1

  [ -z "$DOCKER_HUB_USERNAME" ] && return $ERROR_noDockerHubUsername
  [ -z "$DOCKER_HUB_PASSWORD" ] && return $ERROR_noDockerHubPassword

  docker login --username "$DOCKER_HUB_USERNAME" --password "$DOCKER_HUB_PASSWORD"

  docker push "$DOCKER_HUB_USERNAME/$imageName" || return $ERROR_ship_dockerPush

  echo "$DOCKER_HUB_USERNAME/$imageName" > "$(dirname "${BASH_SOURCE[0]}")"/target/gocdAgentImageName

  docker logout
}



function run() {
  local imageName=$1
  local useBuildServer=$2

  [ -z "$DOCKER_HUB_USERNAME" ] && return $ERROR_noDockerHubUsername

  containerName=${imageName//:/-}
  imageName="$DOCKER_HUB_USERNAME/$imageName"

  if [ "use-build-server" = "$useBuildServer" ] ; then
    goServerUrl="$GO_SERVER"
  else
    goServerUrl="$(cat "$(dirname "${BASH_SOURCE[0]}")"/target/goServerPublicIpAddress)"
  fi

  containerId=$(docker run --name "$containerName" -d \
    -e GO_SERVER="$goServerUrl" \
    "$imageName"
  ) || return $ERROR_run_dockerRun

  docker exec -t "$containerName" /bin/bash /usr/bin/wait_for_go_agent.sh 120 \
  || return $ERROR_run_dockerExec_waitForGoAgent
}



function destroy() {
  local imageName=$1

  containerName=${imageName//:/-}

  docker rm -f "$containerName" \
  || return $ERROR_destroy_dockerRemove_container
}
