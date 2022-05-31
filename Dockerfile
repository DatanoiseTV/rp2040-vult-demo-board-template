FROM ubuntu
LABEL org.opencontainers.image.source="https://github.com/krmnn/rp2040-vult-demo-board-template"

RUN apt-get update && apt-get install -y cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib npm
RUN npm install vult -g
