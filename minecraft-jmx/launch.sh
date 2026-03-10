#!/usr/bin/env bash

javaagent_path="jmx_prometheus_javaagent.jar"

if [ ! -f "${javaagent_path}" ]; then
    curl -L -o "${javaagent_path}" 'https://github.com/prometheus/jmx_exporter/releases/download/1.5.0/jmx_prometheus_javaagent-1.5.0.jar'
fi

JAVA_ARGS=(
    -javaagent:"${javaagent_path}"=8080:jmx_config.yaml
)

exec java "${JAVA_ARGS[@]}" -jar server.jar nogui
