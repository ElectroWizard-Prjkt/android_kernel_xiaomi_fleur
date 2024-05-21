# from image 
FROM ubuntu:24.04

# user
USER root

# setup env
ARG UBUNTU_FRONTEND=noninteractive

# package
RUN apt-get update -qq && \
    apt-get upgrade -y && \
    apt-get install -y bc bison build-essential ccache curl flex glibc-source g++-multilib gcc-multilib binutils-aarch64-linux-gnu \
    git gnupg gperf imagemagick lib32ncurses5-dev lib32readline-dev lib32z1-dev liblz4-tool libncurses-dev libsdl1.2-dev \
    libssl-dev libwxgtk3.0-gtk4-dev libxml2 libxml2-utils lzop pngcrush rsync schedtool squashfs-tools xsltproc zip zlib1g-dev python3 \
    tmate ssh lld ccache cpulimit neofetch

# Set CPU limit and memory limit
RUN cpulimit -i -l 50
RUN ulimit -m 12582912

# Cache and environment configuration
ENV CACHE=1
RUN if [ "$CACHE" -eq 1 ]; then \
        ccache -M 512G && \
        export USE_CCACHE=1; \
    fi
ENV LC_ALL=C

# sudo hax
RUN useradd -l -u 33333 -G sudo -md /home/codespaces -s /usr/bin/bash -p codespaces codespaces \
    && sed -i.bkp -e 's/%sudo\s\+ALL=(ALL\(:ALL\)\?)\s\+ALL/%sudo ALL=NOPASSWD:ALL/g' /etc/sudoers \
    && chmod 0440 /etc/sudoers

# Set default shell to bash
SHELL ["/bin/bash", "-c"]

# Set shell use bash
RUN chsh -s /bin/bash

# env bash
ENV SHELL /bin/bash

# Entry point: start with bash, then switch to fish
ENTRYPOINT ["/bin/bash", "-c"]
