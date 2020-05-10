FROM ubuntu:bionic

RUN apt-get update -y && \
    apt-get install -yq \
    build-essential \
    flex bison \
    python3
