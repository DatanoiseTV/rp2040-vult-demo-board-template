FROM ubuntu
LABEL org.opencontainers.image.source="https://github.com/datanoisetv/rp2040-vult-demo-board-template"

RUN apt-get update && apt-get install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib npm
RUN npm install vult -g
